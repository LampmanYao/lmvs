#include "lmvs-sock.h"
#include "lmvs-log.h"
#include "lmvs-fast.h"
#include "lmvs-utils.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

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
	return sock;
}

void
lmvs_sock_free(lmvs_sock_t* sock) {
	lmvs_rb_free(sock->rb);
	free(sock);
}

int
lmvs_sock_recv(lmvs_sock_t* sock) {
	/* TODO: reduce data copy */
	char recvbuf[READ_BUFFER_SIZE] = {0};
	int nrecv;

tryagain:
	nrecv = recv(sock->fd, recvbuf, READ_BUFFER_SIZE, 0);
	if (lmvs_fast(nrecv > 0)) {
		lmvs_rb_append(sock->rb, recvbuf, nrecv);
		goto tryagain;
	}

	if (nrecv < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		DEBUG("recv error from socket[%d]: %s", sock->fd, strerror(errno));
		return -1; /* error */
	}

	return -1; /* peer closed */
}

int
lmvs_sock_send(lmvs_sock_t* sock, char* buff, int len) {
	int nsend = 0;
	int remain = len;
	while (remain > 0) {
		nsend = send(sock->fd, buff + len - remain, remain, 0);
		if (lmvs_slow(nsend < 0)) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				continue;
			}
			DEBUG("send(%d) error: %s", sock->fd, strerror(errno));
			return -1;
		}
		remain -= nsend;
	}
	return len;
}

