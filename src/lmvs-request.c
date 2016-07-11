#include "lmvs-request.h"
#include "lmvs-response.h"
#include "lmvs-rb.h"
#include "lmvs-log.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define CRLF  "\r\n"
#define CRLF2 "\r\n\r\n"

extern char* root_path;

static char* get_mime_type(char* uri);

lmvs_request_t*
lmvs_request_new() {
	lmvs_request_t* request = calloc(1, sizeof(*request));
	return request;
}

void
lmvs_request_free(lmvs_request_t* request) {
	close(request->fd);
	free(request);
}

int
lmvs_request_parse(lmvs_request_t* request, char* data) {
	if (!strstr(data, CRLF2)) {
		return -1;
	}

	char* p = data;
	request->method = data;

	/* The contents between the begin and the first SP is http method,
	   so, we move p points to the first SP, and set it to '\0' */
	while (*p != ' ') {
		p++;
	}
	*p = '\0';

	/* The contents between the frist SP and the second SP is URI,
	   so, we move p points to the second SP */
	request->uri = p + 1;
	p = request->uri;

	while (*p != ' ') {
		p++;
	}

	*p = '\0';
	p--;

	if (request->uri - p == 0) {
		/* Check uri only is '/' */
		request->uri = "index.html";
	} else {
		request->uri++;
	}
	p++;

	request->mime_type = get_mime_type(request->uri);

	struct stat st_buf;
	char infile[512] = {0};

	snprintf(infile, 512, "%s/%s", root_path, request->uri);
	request->fd = open(infile, O_RDWR, 0644);

	if (request->fd > 0) {
		request->http_status = "HTTP/1.1 200 OK\r\n";
		fstat(request->fd, &st_buf);
		request->content_length = st_buf.st_size;
		lmvs_prepare_response_header(request);
	} else {
		request->http_status = "HTTP/1.1 404 Not Found\r\n";
		request->mime_type = "text/html";
		request->content_length = strlen(notfound_html);
		lmvs_prepare_response_header(request);	
	}

	request->total = request->headers.length + request->content_length;
	request->pdata = request->headers.buffer;
	return 0;
}

void
lmvs_request_reset(lmvs_request_t* request) {
	request->headers.length = 0;
	request->is_headers_sent = HEADERS_NOTSEND;
	request->pdata = NULL;
	request->content_length = 0;
	request->nsend = 0;
	request->total = 0;
	if (request->fd > 0) {
		close(request->fd);
	}
}

static inline char*
get_mime_type(char* uri) {
	/*
	 * .jpg: image/jpeg
	 * .png: image/png
	 * .css: text/css
	 * .html: text/html
	 * .m3u8: application/vnd.apple.mpegurl
	 * .js: application/javascript
	 * .ts: application/octet-stream
	 * .txt: text/plain
	 */

	char* last_dot = strrchr(uri, '.');

	if (!last_dot) {
		return "application/octet-stream";
	}

	if (strcmp(last_dot, ".js") == 0) {
		return "application/javascript";
	} else if (strcmp(last_dot, ".ts") == 0) {
		return "application/octet-stream";
	} else if (strcmp(last_dot, ".jpg") == 0) {
		return "image/jpeg";
	} else if (strcmp(last_dot, ".png") == 0) {
		return "image/png";
	} else if (strcmp(last_dot, ".css") == 0) {
		return "text/css";
	} else if (strcmp(last_dot, ".htm") == 0) {
		return "text/html";
	} else if (strcmp(last_dot, ".txt") == 0) {
		return "text/plain";
	} else if (strcmp(last_dot, ".html") == 0) {
		return "text/html";
	} else if (strcmp(last_dot, ".m3u8") == 0) {
		return "application/vnd.apple.mpegurl";
	} else {
		return "*/*";
	}
}
