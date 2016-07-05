#pragma once

#include "lmvs-rb.h"

typedef struct lmvs_sock {
	int fd;
	unsigned int sid;
	lmvs_rb_t* rb;
} lmvs_sock_t;

lmvs_sock_t* lmvs_sock_new(int rb_size);
void lmvs_sock_free(lmvs_sock_t*);
int lmvs_sock_recv(lmvs_sock_t*);
int lmvs_sock_send(lmvs_sock_t*, char* buff, int len);
int lmvs_sock_sendv(lmvs_sock_t*, char* buff1, int len1, char* buff2, int len2);

