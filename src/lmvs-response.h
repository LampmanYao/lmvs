#pragma once

#include "lmvs-sock.h"
#include "lmvs-request.h"

#define notfound_html \
"<!doctype html><html><html lang=\"en\"><head><meta charset=utf-8/></head><body>404 Not Found</body></html>"

void lmvs_prepare_response_header(lmvs_request_t* request);
int lmvs_response_headers(lmvs_sock_t* sock, lmvs_request_t* request);
int lmvs_response(lmvs_sock_t* sock, lmvs_request_t* request);

