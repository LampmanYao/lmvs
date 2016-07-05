#pragma once

#include "lmvs-sock.h"

typedef struct lmlive_request {
	lmvs_sock_t* sock;
	unsigned int content_length;
	char* method;
	char* http_status;
	char* mime_type;
	char uri[128];
} lmlive_request_t;

lmlive_request_t* lmlive_request_new(lmvs_sock_t* sock);
void lmlive_request_free(lmlive_request_t* request);
void lmlive_request_parse(lmlive_request_t* request);

