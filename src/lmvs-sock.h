#pragma once

#include "lmvs-rb.h"
#include "lmvs-request.h"

typedef struct lmvs_sock {
	int fd;
	unsigned int sid;
	lmvs_rb_t* rb;
	lmvs_request_t* request;
} lmvs_sock_t;

lmvs_sock_t* lmvs_sock_new(int rb_size);
void lmvs_sock_free(lmvs_sock_t*);
int lmvs_sock_recv(lmvs_sock_t*);
int lmvs_sock_send(lmvs_sock_t*, char* buff, int len);
int lmvs_sock_sendfile(lmvs_sock_t*, int fd, unsigned int len);

