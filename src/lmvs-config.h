#pragma once

#include "lmvs-hashtable.h"

typedef struct lmvs_config {
	lmvs_ht_t* hashtbl;
} lmvs_config_t;

lmvs_config_t* lmvs_config_new();
void lmvs_config_free(lmvs_config_t*);
void lmvs_config_load(lmvs_config_t*, const char* file);
void* lmvs_config_find(lmvs_config_t*, void* key, int key_len);

