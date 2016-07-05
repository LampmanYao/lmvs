#include "lmvs-lfhash.h"
#include "lmvs-fast.h"
#include "lmvs-atomic.h"

#include <stdlib.h>

static uint32_t do_hash(uint32_t seed, const unsigned char* str, const ssize_t len);

lmvs_lfhash_t*
lmvs_lfhash_new(int size) {
	lmvs_lfhash_t* ht = calloc(1, sizeof(*ht) + size * sizeof(lmvs_lflist_t*));
	ht->locked = 0;
	lmvs_spinlock_init(&ht->lock);
	ht->size = size;
	ht->seed = random() % UINT32_MAX;
	ht->count = 0;
	for (int i = 0; i < size; i++) {
		ht->table[i] = lmvs_lflist_new();
	}
	return ht;
}

void
lmvs_lfhash_free(lmvs_lfhash_t* ht) {
	for (int i = 0; i < ht->size; i++) {
		lmvs_lflist_free(ht->table[i]);
	}
	free(ht);
}

int
lmvs_lfhash_insert(lmvs_lfhash_t* ht, int64_t key, void* data) {
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	int ret = lmvs_lflist_insert(ht->table[index], key, data);
	if (ret == 0) {
		INC_ONE_ATOMIC(&ht->count);
	}
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_unlock(&ht->lock);
	}
	return ret;
}

void*
lmvs_lfhash_search(lmvs_lfhash_t* ht, int64_t key) {
	void* data = NULL;
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	lmvs_lflist_node_t* node = lmvs_lflist_search(ht->table[index], key);
	if (node) {
		data = node->data;
	}
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_unlock(&ht->lock);
	}
	return data;
}

int lmvs_lfhash_delete(lmvs_lfhash_t* ht, int64_t key) {
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_lock(&ht->lock);
	}
	uint32_t hash = do_hash(ht->seed, (void*)&key, sizeof(int64_t));
	int index = hash % ht->size;
	int ret = lmvs_lflist_delete(ht->table[index], key);
	if (ret == 0) {
		DEC_ONE_ATOMIC(&ht->count);
	}
	if (lmvs_slow(ht->locked)) {
		lmvs_spinlock_unlock(&ht->lock);
	}
	return ret;
}

unsigned long
lmvs_lfhash_count(lmvs_lfhash_t* ht) {
	return INC_N_ATOMIC(&ht->count, 0);
}

lmvs_lflist_t*
lmvs_lfhash_get_all_keys(lmvs_lfhash_t* ht) {
	ht->locked = 1;
	lmvs_spinlock_lock(&ht->lock);
	lmvs_lflist_t* new_list = lmvs_lflist_new();
	for (int i = 0; i < ht->size; i++) {
		lmvs_lflist_t* tmp_list = ht->table[i];
		lmvs_lflist_node_t* tmp_node = tmp_list->head->next;
		for (; tmp_node != tmp_list->tail; tmp_node = tmp_node->next) {
			lmvs_lflist_insert(new_list, tmp_node->key, NULL);
		}
	}
	lmvs_spinlock_unlock(&ht->lock);
	ht->locked = 0;
	return new_list;
}

static inline uint32_t
do_hash(uint32_t seed, const unsigned char* str, const ssize_t len) {
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

