#pragma once

#include <stdint.h>

typedef struct lmvs_htnode lmvs_htnode_t;
typedef struct lmvs_ht lmvs_ht_t;
typedef struct lmvs_htlist lmvs_htlist_t;

struct lmvs_htlist {
	lmvs_htnode_t* head;
	lmvs_htnode_t* tail;
};

struct lmvs_htnode {
	int key_len;
	int value_len;
	void* key;
	void* value;
	lmvs_htnode_t* prev;
	lmvs_htnode_t* next;
};

struct lmvs_ht {
	lmvs_htlist_t** lists;
	uint32_t seed;
	int key_count;
	int size;
};

lmvs_ht_t* lmvs_ht_new();
void lmvs_ht_free(lmvs_ht_t*);
int lmvs_ht_insert(lmvs_ht_t*, void* key, int key_len, void* value, int value_len);
lmvs_htnode_t* lmvs_ht_search(lmvs_ht_t*, void* key, int key_len);

/*
 * lmvs_ht_delete() does not free the memory of node, do it yourself.
 * Both key and vlaue of the node.
 */
int lmvs_ht_delete(lmvs_ht_t*, lmvs_htnode_t* node);

