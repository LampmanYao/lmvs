#pragma once

#include <stdint.h>

typedef struct lmvs_lflist lmvs_lflist_t;
typedef struct lmvs_lflist_node lmvs_lflist_node_t;

struct lmvs_lflist_node {
	int64_t key;
	void* data;
	lmvs_lflist_node_t* next;
};

struct lmvs_lflist {
	lmvs_lflist_node_t* head;
	lmvs_lflist_node_t* tail;
};

lmvs_lflist_node_t* lmvs_lflist_node_new(int64_t key, void* data);
void lmvs_lflist_node_free(lmvs_lflist_node_t* node);

lmvs_lflist_t* lmvs_lflist_new();
void lmvs_lflist_free(lmvs_lflist_t*);
int lmvs_lflist_insert(lmvs_lflist_t*, int64_t key, void* data);
int lmvs_lflist_delete(lmvs_lflist_t*, int64_t key);
lmvs_lflist_node_t* lmvs_lflist_search(lmvs_lflist_t*, int64_t key);

