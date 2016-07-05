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

extern char* root_path;

lmlive_request_t*
lmlive_request_new(lmvs_sock_t* sock) {
	lmlive_request_t* request = calloc(1, sizeof(*request));
	request->sock = sock;
	return request;
}

void
lmlive_request_free(lmlive_request_t* request) {
	free(request);
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

static inline char*
http_method(char* method) {
	if (strcmp(method, "GET") == 0) {
		return "GET";
	} else if (strcmp(method, "POST") == 0) {
		return "POST";
	} else if (strcmp(method, "DELETE") == 0) {
		return "DELETE";
	} else if (strcmp(method, "OPTION") == 0) {
		return "OPTION";
	} else if (strcmp(method, "PUT") == 0) {
		return "PUT";
	} else if (strcmp(method, "HEAD") == 0) {
		return "HEAD";
	} else {
		return "UNKNOWN";
	}
}

void
lmlive_request_parse(lmlive_request_t* request) {
	char* data;
	char* p;
	char* method;
	char* uri;

	data = lmvs_rb_data(request->sock->rb);
	method = data;
	p = method;

	while (*p != ' ') {
		p++;
	}
	*p = '\0';

	request->method = http_method(method);
	*p  = ' ';

	uri = p + 1;
	p = uri;

	while (*p != ' ') {
		p++;
	}
	*p = '\0';
	p--;

	if (uri - p == 0) {
		uri++;
		strcpy(request->uri, "index.html");
	} else {
		uri++;
		strcpy(request->uri, uri);
	}

	p++;
	*p = ' ';
	request->mime_type = get_mime_type(request->uri);

	lmvs_headers_t headers = {.length = 0};
	int in_fd;
	struct stat buf;
	char* buffer = NULL;
	char infile[512] = {0};

	sprintf(infile, "%s/%s", root_path, request->uri);
	in_fd = open(infile, O_RDWR, 0644);

	if (in_fd > 0) {
		request->http_status = "HTTP/1.1 200 OK\r\n";
		fstat(in_fd, &buf);
		buffer = malloc(buf.st_size);
		request->content_length = read(in_fd, buffer, buf.st_size);
		lmvs_prepare_response_header(&headers, request);
		lmvs_sock_send(request->sock, headers.buffer, headers.length);
		lmvs_sock_send(request->sock, buffer, request->content_length);
		free(buffer);
		close(in_fd);
	} else {
		request->http_status = "HTTP/1.1 404 Not Found\r\n";
		request->mime_type = "text/html";
		request->content_length = strlen(notfound_html);
		lmvs_prepare_response_header(&headers, request);
		lmvs_sock_send(request->sock, headers.buffer, headers.length);
		lmvs_sock_send(request->sock, notfound_html, request->content_length);
	}
}

