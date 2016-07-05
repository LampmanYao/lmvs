#pragma once

#include <pthread.h>

#define CSNET_COND_INITILIAZER              \
{                                           \
	.mutex = PTHREAD_MUTEX_INITIALIZER, \
	.cond = PTHREAD_COND_INITIALIZER    \
}

typedef struct lmvs_cond {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} lmvs_cond_t;


int lmvs_cond_init(lmvs_cond_t*);
void lmvs_cond_destroy(lmvs_cond_t*);
void lmvs_cond_wait(lmvs_cond_t*);
void lmvs_cond_wait_sec(lmvs_cond_t*, int second);
void lmvs_cond_signal_one(lmvs_cond_t*);
void lmvs_cond_signal_all(lmvs_cond_t*);

