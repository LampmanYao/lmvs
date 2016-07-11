#pragma once

#include <sys/stat.h>

#define HEADERS_NOTSEND 0
#define HEADERS_HASSEND 1

typedef struct lmvs_headers {
	int length;
	char buffer[1024];
} lmvs_headers_t;

typedef struct lmvs_request {
	lmvs_headers_t headers;
	int is_headers_sent;
	char* method;
	char* http_status;
	char* mime_type;
	char* uri;
	char* pdata;
	int fd;
	int content_length;
	int nsend;
	int total;
} lmvs_request_t;

lmvs_request_t* lmvs_request_new();
void lmvs_request_free(lmvs_request_t* request);
void lmvs_request_reset(lmvs_request_t* request);
int lmvs_request_parse(lmvs_request_t* request, char* data);

