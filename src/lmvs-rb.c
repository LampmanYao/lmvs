#include "lmvs-rb.h"
#include "lmvs-utils.h"

#include <stdlib.h>
#include <string.h>

#ifdef JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#define MAX_BUFFER_LEN 1024 * 1024 * 5  /* 5M */

static inline char*
_expand(int size) {
	if (size > MAX_BUFFER_LEN) {
		return NULL;
	}

	return calloc(1, size);
}

lmvs_rb_t*
lmvs_rb_new(unsigned int size) {
	lmvs_rb_t* rb = calloc(1, sizeof(*rb));
	if (!rb) {
		lmvs_oom(sizeof(*rb));
	}

	rb->capacity = size;
	rb->data_len = 0;
	rb->seek = 0;

	rb->buffer = calloc(1, size);
	if (!rb->buffer) {
		lmvs_oom(size);
	}

	return rb;
}

void
lmvs_rb_free(lmvs_rb_t* rb) {
	free(rb->buffer);
	free(rb);
}

int
lmvs_rb_append(lmvs_rb_t* rb, const char* data, unsigned int len) {
	if ((rb->data_len == 0) && (rb->seek == 0)) {
		memcpy(rb->buffer, data, len);
		rb->data_len = len;
	} else {
		unsigned int remain = rb->capacity - rb->data_len;
		if (remain >= len) {
			if (rb->capacity - (rb->seek + rb->data_len) >= len) {
				memcpy(rb->buffer + rb->seek + rb->data_len, data, len);
				rb->data_len += len;
			} else {
				memmove(rb->buffer, rb->buffer + rb->seek, rb->data_len);
				memcpy(rb->buffer + rb->data_len, data, len);
				rb->data_len += len;
				rb->seek = 0;
			}
		} else {
			char* new_buffer = _expand(rb->capacity * 2);
			if (!new_buffer) {
				return -1;
			}

			rb->capacity *= 2;
			memcpy(new_buffer, rb->buffer + rb->seek, rb->data_len);
			memcpy(new_buffer+ rb->data_len, data, len);
			free(rb->buffer);
			rb->buffer = new_buffer;
			rb->data_len += len;
			rb->seek = 0;
		}
	}

	return 0;
}

unsigned int
lmvs_rb_seek(lmvs_rb_t* rb, unsigned int len) {
	if (len > rb->data_len) {
		/* TODO: data_len and seek should be reset ? */
		return 0;
	}
	rb->data_len -= len;
	if (rb->data_len == 0) {
		rb->seek = 0;
	} else {
		rb->seek += len;
	}
	return rb->data_len;
}

inline char*
lmvs_rb_data(lmvs_rb_t* rb) {
	return rb->buffer + rb->seek;
}

inline unsigned int
lmvs_rb_data_len(lmvs_rb_t* rb) {
	return rb->data_len;
}

void
lmvs_rb_reset(lmvs_rb_t* rb) {
	rb->data_len = 0;
	rb->seek = 0;
}

