#pragma once

#include "lmvs-lflist.h"
#include "lmvs-spinlock.h"

#include <stdint.h>

typedef struct lmvs_lfhash lmvs_lfhash_t;

struct lmvs_lfhash {
	unsigned int locked;
	lmvs_spinlock_t lock;
	int size;
	uint32_t seed;
	unsigned long count;
	lmvs_lflist_t* table[0];
};

lmvs_lfhash_t* lmvs_lfhash_new(int size);
void lmvs_lfhash_free(lmvs_lfhash_t*);
int lmvs_lfhash_insert(lmvs_lfhash_t*, int64_t key, void* data);
void* lmvs_lfhash_search(lmvs_lfhash_t*, int64_t key);
int lmvs_lfhash_delete(lmvs_lfhash_t*, int64_t key);
unsigned long lmvs_lfhash_count(lmvs_lfhash_t*);
lmvs_lflist_t* lmvs_lfhash_get_all_keys(lmvs_lfhash_t*);

