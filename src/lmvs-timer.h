#pragma once

#include "lmvs-lfhash.h"

typedef struct lmvs_timer_node {
	int fd;
	unsigned int timerid;
	int interval;  /* TODO: We can use this value to reduce moving.
	                  When one lmvs_timer recv data, we don't need move this
			  lmvs_timer to another slot, we just decrease this value */
} lmvs_timer_node_t;

typedef struct lmvs_timer {
	int interval;
	int prev_wheel;
	int curr_wheel;
	unsigned long curr_time;
	lmvs_lfhash_t* which_wheel_tbl;
	lmvs_lfhash_t* wheels_tbl[0];
} lmvs_timer_t;

lmvs_timer_t* lmvs_timer_new(int interval, int wheel_count);
void lmvs_timer_free(lmvs_timer_t*);
int lmvs_timer_insert(lmvs_timer_t*, int fd, unsigned int sid);
void lmvs_timer_remove(lmvs_timer_t*, unsigned int timerid);
void lmvs_timer_update(lmvs_timer_t*, unsigned int timerid);

/* Return -1 means there is no expired timer,
 * other value means there is expired timer in this wheel */
int lmvs_timer_book_keeping(lmvs_timer_t*);

