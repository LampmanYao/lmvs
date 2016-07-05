#pragma once

#include "lmvs-request.h"

#define notfound_html \
"<!doctype html><html><html lang=\"en\"><head><meta charset=utf-8/></head><body>404 Not Found</body></html>"

typedef struct lmvs_headers {
	int length;
	char buffer[1024];
} lmvs_headers_t;

void lmvs_prepare_response_header(lmvs_headers_t* headers, lmvs_request_t* request);

