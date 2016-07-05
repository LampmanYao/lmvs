#pragma once

#include "lmvs-sock.h"

typedef struct lmvs_sockset {
	int max_conn;
	unsigned int start_sid;
	unsigned int curr_sid;
	lmvs_sock_t* set[0];
} lmvs_sockset_t;

lmvs_sockset_t* lmvs_sockset_new(int max_conn, unsigned int start_sid);
void lmvs_sockset_free(lmvs_sockset_t*);
unsigned int lmvs_sockset_put(lmvs_sockset_t*, int fd);
lmvs_sock_t* lmvs_sockset_get(lmvs_sockset_t*, unsigned int sid);
void lmvs_sockset_reset_sock(lmvs_sockset_t*, unsigned int fd);

