#include "lmvs-response.h"
#include "lmvs-utils.h"

#include <stdio.h>
#include <string.h>

#define APPEND_STRING(str) \
	do { \
		int len = strlen(str); \
		memcpy((headers->buffer + headers->length), (str), (len)); \
		headers->length += len; \
	} while (0)

void
lmvs_prepare_response_header(lmvs_headers_t* headers, lmvs_request_t* request) {
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

