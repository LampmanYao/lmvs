#include "lmvs.h"
#include "lmvs-el.h"
#include "lmvs-fast.h"
#include "lmvs-utils.h"
#include "lmvs-sock.h"
#include "lmvs-sockset.h"
#include "lmvs-socket-api.h"
#include "lmvs-request.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

static void lmvs_el_check_timeout(lmvs_el_t* el);

lmvs_el_t*
lmvs_el_new(int max_conn, int connect_timeout, lmvs_log_t* log) {
	lmvs_el_t* el = calloc(1, sizeof(*el));
	if (!el) {
		lmvs_oom(sizeof(*el));
	}

	el->max_conn = max_conn;
	el->cur_conn = 0;
	el->epoller = lmvs_epoller_new(max_conn);
	el->sockset = lmvs_sockset_new(max_conn, 0xab);
	el->timer = lmvs_timer_new(connect_timeout, 709);
	el->log = log;

	return el;
}

int
lmvs_el_add_connection(lmvs_el_t* el, int fd) {
	unsigned int sid;
	if (lmvs_slow(el->cur_conn++ > el->max_conn)) {
		LOG_WARNING(el->log, "Too much connections, closing socket %d", fd);
		return -1;
	}

	sid = lmvs_sockset_put(el->sockset, fd);
	lmvs_epoller_add(el->epoller, fd, sid);
	lmvs_timer_insert(el->timer, fd, sid);

	return 0;
}

void*
lmvs_el_io_loop(void* arg) {
	lmvs_el_t* el = (lmvs_el_t*)arg;

	while (1) {
		int ready = lmvs_epoller_wait(el->epoller, 1000);
		for (int i = 0; i < ready; ++i) {
			lmvs_epoller_event_t* ee;
			lmvs_sock_t* sock;
			unsigned int sid;
			int fd;

			ee = lmvs_epoller_get_event(el->epoller, i);
			sid = lmvs_epoller_event_sid(ee);
			sock = lmvs_sockset_get(el->sockset, sid);
			fd = sock->fd;

			if (lmvs_fast(lmvs_epoller_event_is_readable(ee))) {
				int nrecv = lmvs_sock_recv(sock);
				if (lmvs_fast(nrecv == 0)) {
					unsigned int data_len = lmvs_rb_data_len(sock->rb);
					lmlive_request_t* request = lmlive_request_new(sock);
					lmlive_request_parse(request);
					lmvs_rb_seek(sock->rb, data_len);
				} else {
					/* We clear a socket here. Because of if remote peer close immediately
					 * after remote peer send a large amout of data, we can still read() data
					 * from this socket. */
					lmvs_epoller_del(el->epoller, fd, sid);
					lmvs_sockset_reset_sock(el->sockset, sid);
					lmvs_timer_remove(el->timer, sid);
					el->cur_conn--;
				}
			}

			if (lmvs_epoller_event_is_error(ee)) {
				/* EPOLLERR and EPOLLHUP events can occur if the remote peer
				 * was colsed or a terminal hangup occured. We do nothing
				 * here but LOGGING. */
				LOG_WARNING(el->log, "EPOLLERR on socket: %d. ", fd);
			}
		} /* end of for (ready) */

		/* Check the connection which has timeout. */
		lmvs_el_check_timeout(el);

		if (ready == -1) {
			if (errno == EINTR) {
				/* Stopped by a signal */
				continue;
			} else {
				LOG_ERROR(el->log, "epoll_wait(): %s", strerror(errno));
				return NULL;
			}
		}
	} /* end of while (1) */

	return NULL;
}

void
lmvs_el_free(lmvs_el_t* el) {
	lmvs_epoller_free(el->epoller);
	lmvs_sockset_free(el->sockset);
	lmvs_timer_free(el->timer);
}

static inline void
lmvs_el_check_timeout(lmvs_el_t* el) {
	int expired_wheel = lmvs_timer_book_keeping(el->timer);
	if (expired_wheel > -1) {
		lmvs_lfhash_t* hashtbl = el->timer->wheels_tbl[expired_wheel];
		lmvs_lflist_t* keys = lmvs_lfhash_get_all_keys(hashtbl);
		lmvs_lflist_node_t* head = keys->head->next;
		while (head != keys->tail) {
			/* We just close this connection simply here.
			 * TODO: Shoud send a timeout package to this connection?
			 * If we do this, what's the right timing to close this connection? */

			unsigned int sid = head->key;
			lmvs_sock_t* sock = lmvs_sockset_get(el->sockset, sid);
			LOG_WARNING(el->log, "sid[%d] timeout, closing socket[%d]", sid, sock->fd);
			lmvs_epoller_del(el->epoller, sock->fd, sid);
			lmvs_sockset_reset_sock(el->sockset, sid);
			el->cur_conn--;
			lmvs_timer_remove(el->timer, sid);
			head = head->next;
		}
		lmvs_lflist_free(keys);
	}
}

