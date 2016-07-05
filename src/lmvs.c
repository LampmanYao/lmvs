#include "lmvs.h"
#include "lmvs-cond.h"
#include "lmvs-fast.h"
#include "lmvs-utils.h"
#include "lmvs-socket-api.h"
#include "lmvs-response.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define MAGIC_NUMBER 1024

static void _do_accept(lmvs_t* lmvs, int* listenfd);

lmvs_t*
lmvs_new(int port, int thread_count, int max_conn, int connect_timeout,
	lmvs_log_t* log)
{
	lmvs_t* lmvs;
	lmvs = calloc(1, sizeof(*lmvs) + thread_count * sizeof(lmvs_el_t*));
	if (!lmvs) {
		lmvs_oom(sizeof(*lmvs));
	}
	lmvs->listenfd = listen_port(port);
	if (lmvs->listenfd == -1) {
		LOG_FATAL(log, "epoll_create(): %s", strerror(errno));
	}

	if (set_nonblocking(lmvs->listenfd) == -1) {
		LOG_FATAL(log, "cannot set socket: %d to nonblock", lmvs->listenfd);
	}

	lmvs->cpu_cores = lmvs_cpu_cores();
	lmvs->thread_count = thread_count;
	lmvs->max_conn = max_conn;
	lmvs->epoller = lmvs_epoller_new(MAGIC_NUMBER);

	if (!lmvs->epoller) {
		LOG_FATAL(log, "epoll_create(): %s", strerror(errno));
	}

	if (lmvs_epoller_add(lmvs->epoller, lmvs->listenfd, 0) == -1) {
		LOG_FATAL(log, "epoll_ctl(): %s", strerror(errno));
	}

	for (int i = 0; i < thread_count; i++) {
		int count = max_conn / thread_count + 1;
		lmvs->el_list[i] = lmvs_el_new(count, connect_timeout, log);
	}

	lmvs->log = log;
	return lmvs;
}

void
lmvs_loop(lmvs_t* lmvs, int timeout) {
	for (int i = 0; i < lmvs->thread_count; i++) {
		int cpuid;
		pthread_t tid;

		if (pthread_create(&tid, NULL, lmvs_el_io_loop, lmvs->el_list[i]) < 0) {
			LOG_FATAL(lmvs->log, "create lmvs_el_io_loop() error. pthread_create(): %s", strerror(errno));
		}

		if (lmvs->cpu_cores > 4) {
			/* Skip CPU0 and CPU1 */
			cpuid = i + 2 % lmvs->cpu_cores;
			if (i >= lmvs->cpu_cores - 2) {
				cpuid = i + 4 - lmvs->cpu_cores;
			}

			lmvs_bind_to_cpu(tid, cpuid);
		} else {
			cpuid = i % lmvs->cpu_cores;
			lmvs_bind_to_cpu(tid, cpuid);
		}
	}

	while (1) {
		int ready = lmvs_epoller_wait(lmvs->epoller, timeout);
		for (int i = 0; i < ready; ++i) {
			lmvs_epoller_event_t* ee = lmvs_epoller_get_event(lmvs->epoller, i);
			int fd = lmvs_epoller_event_fd(ee);

			if (lmvs_epoller_event_is_readable(ee)) {
				if (fd == lmvs->listenfd) {
					/* Have a notification on the listening socket,
					   which means one or more new incoming connecttions */
					_do_accept(lmvs, &lmvs->listenfd);
				}
			}

			if (lmvs_epoller_event_is_error(ee)) {
				LOG_ERROR(lmvs->log, "epoll event error");
				close(fd);
				continue;
			}
		}

		if (ready == -1) {
			if (errno == EINTR) {  /* Stopped by a signal */
				continue;
			} else {
				LOG_ERROR(lmvs->log, "epoll_wait(): %s", strerror(errno));
				return;
			}
		}
	}
}

void
lmvs_free(lmvs_t* lmvs) {
	close(lmvs->listenfd);
	lmvs_epoller_free(lmvs->epoller);
	for (int i = 0; i < lmvs->thread_count; i++) {
		lmvs_el_free(lmvs->el_list[i]);
	}
	free(lmvs);
}

static inline void
_do_accept(lmvs_t* lmvs, int* listenfd) {
	while (1) {
		int fd;
		struct sockaddr_in sin;
		socklen_t len = sizeof(struct sockaddr_in);
		bzero(&sin, len);
		fd = accept(*listenfd, (struct sockaddr*)&sin, &len);

		if (fd > 0) {
			LOG_INFO(lmvs->log, "accept incoming [%s:%d] with socket: %d.",
				inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), fd);

			if (set_nonblocking(fd) == -1) {
				LOG_ERROR(lmvs->log, "can not set socket: %d to nonblock", fd);
				close(fd);
				continue;
			}

			if (lmvs_el_add_connection(lmvs->el_list[fd % lmvs->thread_count], fd) == -1) {
				close(fd);
				return;
			}
		} else {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				/* We have processed all incoming connections. */
				return;
			} else {
				LOG_ERROR(lmvs->log, "accept(): %s", strerror(errno));
				return;
			}
		}
	}
}

