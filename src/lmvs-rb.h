#pragma once

typedef struct lmvs_rb {
	unsigned int capacity;
	unsigned int data_len;
	unsigned int seek;
	char* buffer;
} lmvs_rb_t;

lmvs_rb_t* lmvs_rb_new(unsigned int size);
void lmvs_rb_free(lmvs_rb_t*);
int lmvs_rb_append(lmvs_rb_t*, const char* data, unsigned int len);
unsigned int lmvs_rb_seek(lmvs_rb_t*, unsigned int len);
void lmvs_rb_reset(lmvs_rb_t*);
char* lmvs_rb_data(lmvs_rb_t*);
unsigned int lmvs_rb_data_len(lmvs_rb_t*);

