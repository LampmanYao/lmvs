#include "lmvs-sockset.h"
#include "lmvs-utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

static inline unsigned int
SOCKID(lmvs_sockset_t* set) { 
	if (++set->curr_sid == 0xbabeface) {
		set->curr_sid = set->start_sid;
	}
	return set->curr_sid;
}

lmvs_sockset_t*
lmvs_sockset_new(int max_conn, unsigned int start_sid) {
	lmvs_sockset_t* set = calloc(1, sizeof(*set) + max_conn * sizeof(lmvs_sock_t*));
	if (!set) {
		lmvs_oom(sizeof(*set));
	}

	set->start_sid = start_sid;
	set->curr_sid = start_sid;
	set->max_conn = max_conn;
	for (int i = 0; i < max_conn; i++) {
		lmvs_sock_t* sock = lmvs_sock_new(4096);
		set->set[i] = sock;
	}

	return set;
}

void
lmvs_sockset_free(lmvs_sockset_t* set) {
	for (int i = 0; i < set->max_conn; i++) {
		lmvs_sock_free(set->set[i]);
	}
	free(set);
}

unsigned int
lmvs_sockset_put(lmvs_sockset_t* set, int fd) {
	int count = set->max_conn;
	unsigned int sid = SOCKID(set);
	lmvs_sock_t* sock = set->set[sid % set->max_conn];
	while (sock->sid != 0 && --count > 0) {
		sid = SOCKID(set);
		sock = set->set[sid % set->max_conn];
	}
	sock->sid = sid;
	sock->fd = fd;
	return sid;
}

lmvs_sock_t*
lmvs_sockset_get(lmvs_sockset_t* set, unsigned int sid) {
	return set->set[sid % set->max_conn];
}

void
lmvs_sockset_reset_sock(lmvs_sockset_t* set, unsigned int sid) {
	lmvs_sock_t* sock = set->set[sid % set->max_conn];
	if (sock->sid == sid) {
		sock->sid = 0;
		close(sock->fd);
		lmvs_rb_reset(sock->rb);
	}
}

