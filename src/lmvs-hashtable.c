#include "lmvs-hashtable.h"

#include <stdio.h>
#include <stdlib.h>  /* calloc and free */
#include <string.h>  /* memcpy */
#include <assert.h>

/*
 * TODO: resize the hash table. Load factor.
 */

static uint32_t _hash(uint32_t seed, const unsigned char* str, const ssize_t len);
static lmvs_htlist_t* lmvs_htlist_new();
static void lmvs_htlist_free(lmvs_htlist_t* l);
static void lmvs_htlist_insert(lmvs_htlist_t* l, lmvs_htnode_t* node);
static void lmvs_htlist_delete(lmvs_htlist_t* l, lmvs_htnode_t* node);

lmvs_ht_t*
lmvs_ht_new() {
	lmvs_ht_t* table = calloc(1, sizeof(*table));
	table->size = 709;
	table->key_count = 0;
	table->seed = random() % UINT32_MAX;
	table->lists = (lmvs_htlist_t**)calloc(table->size, sizeof(lmvs_htlist_t*));

	for (int i = 0; i < table->size; i++) {
		table->lists[i] = lmvs_htlist_new();
	}

	return table;
}

void
lmvs_ht_free(lmvs_ht_t* table) {
	int size = table->size;
	for (int i = 0; i < size; i++) {
		lmvs_htlist_free(table->lists[i]);
	}
	free(table->lists);
	free(table);
}

int
lmvs_ht_insert(lmvs_ht_t* table, void* key, int key_len, void* value, int value_len) {
	uint32_t hash = _hash(table->seed, key, key_len);
	int index = hash % table->size;

	lmvs_htnode_t* node = calloc(1, sizeof(*node));
	node->key = key;
	node->value = value;
	node->key_len = key_len;
	node->value_len = value_len;
	node->next = NULL;

	lmvs_htlist_insert(table->lists[index], node);
	table->key_count++;

	return 0;
}

lmvs_htnode_t*
lmvs_ht_search(lmvs_ht_t* table, void* key, int key_len) {
	uint32_t hash = _hash(table->seed, key, key_len);
	int index = hash % table->size;

	if (!table->lists[index]) {
		return NULL;
	}

	lmvs_htnode_t* temp = table->lists[index]->head;

	while (temp) {
		while (temp && temp->key_len != key_len) {
			temp = temp->next;
		}

		if (temp) {
			if (!memcmp(temp->key, key, key_len)) {
				return temp;
			} else {
				temp = temp->next;
			}
		}
	}

	return NULL;
}

int
lmvs_ht_delete(lmvs_ht_t* table, lmvs_htnode_t* node) {
	uint32_t hash = _hash(table->seed, node->key, node->key_len);
	int index = hash % table->size;

	if (!table->lists[index]) {
		return -1; 
	}

	lmvs_htlist_delete(table->lists[index], node);
	return 0;
}

static inline uint32_t
_hash(uint32_t seed, const unsigned char* str, const ssize_t len) {
	uint32_t hash = seed + len;
	ssize_t slen = len;

	while (slen--) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	return (hash + (hash << 15));
}

static inline lmvs_htlist_t*
lmvs_htlist_new() {
	lmvs_htlist_t* l = calloc(1, sizeof(*l));
	l->head = NULL;
	l->tail = NULL;
	return l;
}

static inline void
lmvs_htlist_free(lmvs_htlist_t* l) {
	lmvs_htnode_t* x = l->head;
	while (x) {
		lmvs_htnode_t* tmp = x->next;
		free(x);
		x = tmp;
	}
	free(l);
}

static inline void
lmvs_htlist_insert(lmvs_htlist_t* l, lmvs_htnode_t* node) {
	node->next = l->head;
	if (l->head) {
		l->head->prev = node;
	}
	l->head = node;
	node->prev = NULL;
}

static inline void
lmvs_htlist_delete(lmvs_htlist_t* l, lmvs_htnode_t* node) {
	if (node->prev) {
		node->prev->next = node->next;
	} else {
		l->head = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}
}

