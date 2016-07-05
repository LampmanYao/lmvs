#pragma once

#include "lmvs-sock.h"

typedef struct lmvs_request {
	lmvs_sock_t* sock;
	unsigned int content_length;
	char* method;
	char* http_status;
	char* mime_type;
	char uri[128];
} lmvs_request_t;

lmvs_request_t* lmvs_request_new(lmvs_sock_t* sock);
void lmvs_request_free(lmvs_request_t* request);
void lmvs_request_parse(lmvs_request_t* request);

