#include "lmvs-sock.h"
#include "lmvs-log.h"
#include "lmvs-fast.h"
#include "lmvs-utils.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define READ_BUFFER_SIZE  (4096)

lmvs_sock_t*
lmvs_sock_new(int rsize) {
	lmvs_sock_t* sock = calloc(1, sizeof(*sock));
	if (lmvs_slow(!sock)) {
		lmvs_oom(sizeof(*sock));
	}
	sock->fd = 0;
	sock->sid = 0;
	sock->rb = lmvs_rb_new(rsize);
	sock->request = lmvs_request_new();
	return sock;
}

void
lmvs_sock_free(lmvs_sock_t* sock) {
	lmvs_rb_free(sock->rb);
	lmvs_request_free(sock->request);
	free(sock);
}

int
lmvs_sock_recv(lmvs_sock_t* sock) {
	char recvbuf[READ_BUFFER_SIZE];
	int nrecv;

tryagain:
	nrecv = recv(sock->fd, recvbuf, READ_BUFFER_SIZE, 0);
	if (lmvs_fast(nrecv > 0)) {
		lmvs_rb_append(sock->rb, recvbuf, nrecv);
		goto tryagain;
	}

	if (nrecv < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return sock->rb->data_len;
		}
		DEBUG("recv error from socket(%d): %s", sock->fd, strerror(errno));
		return -1; /* error */
	}

	return -1; /* peer closed */
}

/*
 * Return value:
 *    -1 : error
 *     0 : socket buffer is full
 *     1 : success
 */
int
lmvs_sock_send(lmvs_sock_t* sock, char* buff, int len) {
	int nsend = send(sock->fd, buff, len, 0);
	if (lmvs_fast(nsend > 0)) {
		return nsend;
	} else {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		DEBUG("send to socket(%d) error: %s", sock->fd, strerror(errno));
		return -1;
	}
}

int
lmvs_sock_sendfile(lmvs_sock_t* sock, int fd, unsigned int len) {
	unsigned int nsend = 0;
	while (1) {
		int ret = sendfile(sock->fd, fd, NULL, len);
		if (ret > 0) {
			nsend += ret;
			if (nsend == len) {
				return nsend;
			} else {
				continue;
			}
		} else {
			if (errno == EAGAIN) {
				return nsend;
			}
			return -1;
		}
	}
}

