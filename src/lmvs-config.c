#include "lmvs-config.h"
#include "lmvs-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

lmvs_config_t*
lmvs_config_new() {
	lmvs_config_t* conf = calloc(1, sizeof(*conf));
	if (!conf) {
		lmvs_oom(sizeof(*conf));
	}
	conf->hashtbl = lmvs_ht_new();
	return conf;
}

void
lmvs_config_free(lmvs_config_t* conf) {
	lmvs_ht_free(conf->hashtbl);
	free(conf);
}

void
lmvs_config_load(lmvs_config_t* conf, const char* file) {
	FILE* f = fopen(file, "r");
	assert(f != NULL);

	char line[512] = {0};
	while (fgets(line, 512, f)) {
		if (line[0] == '#' || line[0] == '\n') {
			continue;
		}

		line[strlen(line) - 1] = '\0';
		char* p = strchr(line, '=');
		if (!p) {
			fprintf(stderr, "WARNING: `%s` line does no contain '=' in `%s`\n", line, file);
			fflush(stderr);
			continue;
		}

		*p = '\0';

		char* tmp1 = lmvs_trim(line);
		char* tmp2 = lmvs_trim(p + 1);
		int key_len = strlen(tmp1);
		int value_len = strlen(tmp2);
		char* key = calloc(1, key_len);
		char* value = calloc(1, value_len);

		if (!key) {
			lmvs_oom(key_len);
		}

		if (!value) {
			lmvs_oom(value_len);
		}

		strcpy(key, tmp1);
		strcpy(value, tmp2);
		lmvs_ht_insert(conf->hashtbl, key, key_len, value, value_len);
	}

	fclose(f);
}

void*
lmvs_config_find(lmvs_config_t* conf, void* key, int key_len) {
	if (!key) {
		return NULL;
	}

	lmvs_htnode_t* htnode = lmvs_ht_search(conf->hashtbl, key, key_len);
	if (htnode) {
		return htnode->value;
	} else {
		return NULL;
	}
}

