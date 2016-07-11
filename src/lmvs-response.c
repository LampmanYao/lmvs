#include "lmvs-response.h"
#include "lmvs-utils.h"
#include "lmvs-fast.h"
#include "lmvs-log.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define APPEND_STRING(str) \
	do { \
		int len = strlen(str); \
		memcpy((request->headers.buffer + request->headers.length), (str), (len)); \
		request->headers.length += len; \
	} while (0)

void
lmvs_prepare_response_header(lmvs_request_t* request) {
	char content_length[32];
	char date_buffer[64];

	APPEND_STRING(request->http_status);
	APPEND_STRING("Content-Type: ");
	APPEND_STRING(request->mime_type);
	APPEND_STRING("\r\nConnection: keep-alive\r\n");
	sprintf(content_length, "Content-Length: %u\r\n", request->content_length);
	APPEND_STRING(content_length);
	http_date(date_buffer, 64);
	APPEND_STRING(date_buffer);
	APPEND_STRING("Cache-Control: max-age=7200, must-revalidate\r\n");
	APPEND_STRING("Server: lmvs\r\n\r\n\0");
}

int
lmvs_response(lmvs_sock_t* sock, lmvs_request_t* request) {
	int headers_nsend = 0;
	int content_nsend = 0;

	if (request->is_headers_sent == HEADERS_NOTSEND) {
		headers_nsend = lmvs_sock_send(sock, request->pdata, request->headers.length);
		if (headers_nsend == -1) {
			return -1;
		}

		if (headers_nsend < request->headers.length) {
			request->pdata += headers_nsend;
			return headers_nsend;
		} else {
			request->is_headers_sent = HEADERS_HASSEND;
			request->pdata = notfound_html;
		}
	}

	if (lmvs_slow(request->fd <= 0)) {
		content_nsend = lmvs_sock_send(sock, request->pdata, request->content_length);
		if (content_nsend == -1) {
			return -1;
		}

		if (content_nsend < request->content_length) {
			request->pdata += content_nsend;
			return content_nsend;
		}

		return content_nsend + headers_nsend;
	} else {
		content_nsend = lmvs_sock_sendfile(sock, request->fd, request->content_length);
		if (content_nsend > 0) {
			return content_nsend + headers_nsend;
		} else {
			return -1;
		}
	}
}

