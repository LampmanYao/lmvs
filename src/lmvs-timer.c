#include "lmvs-timer.h"
#include "lmvs-log.h"
#include "lmvs-fast.h"
#include "lmvs-utils.h"

#include <stdlib.h>

lmvs_timer_t*
lmvs_timer_new(int interval, int wheel_count) {
	lmvs_timer_t* timer = (lmvs_timer_t*)calloc(1, sizeof(*timer) + (interval + 1) * sizeof(lmvs_lfhash_t*));
	if (!timer) {
		lmvs_oom(sizeof(*timer));
	}

	timer->interval = interval;
	timer->prev_wheel = interval;
	timer->curr_wheel = 0;
	timer->curr_time = lmvs_gettime();
	timer->which_wheel_tbl = lmvs_lfhash_new(wheel_count * 2);
	for (int i = 0; i < interval + 1; i++) {
		timer->wheels_tbl[i] = lmvs_lfhash_new(wheel_count);
	}

	return timer;
}

void
lmvs_timer_free(lmvs_timer_t* timer) {
	lmvs_lfhash_free(timer->which_wheel_tbl);
	for (int i = 0; i < timer->interval + 1; i++) {
		lmvs_lfhash_free(timer->wheels_tbl[i]);
	}
	free(timer);
}

int
lmvs_timer_insert(lmvs_timer_t* timer, int fd, unsigned int sid) {
	int* which_wheel = calloc(1, sizeof(int));
	if (!which_wheel) {
		lmvs_oom(sizeof(int));
	}

	*which_wheel = timer->prev_wheel;
	if (lmvs_lfhash_insert(timer->which_wheel_tbl, sid, which_wheel) == -1) {
		free(which_wheel);
		return -1;
	}

	lmvs_timer_node_t* node = (lmvs_timer_node_t*)calloc(1, sizeof(*node));
	if (lmvs_slow(!node)) {
		lmvs_oom(sizeof(*node));
	}

	node->fd = fd;
	node->timerid = sid;
	node->interval = timer->interval;

	if (lmvs_lfhash_insert(timer->wheels_tbl[*which_wheel], sid, node) == -1) {
		lmvs_lfhash_search(timer->which_wheel_tbl, sid);
		free(node);
		return -1;
	}

	return 0;
}

void
lmvs_timer_update(lmvs_timer_t* timer, unsigned int timerid) {
	int* which_wheel = lmvs_lfhash_search(timer->which_wheel_tbl, timerid);
	if ((!which_wheel) || (*which_wheel == timer->prev_wheel)) {
		return;
	}

	lmvs_timer_node_t* timer_node = lmvs_lfhash_search(timer->wheels_tbl[*which_wheel], timerid);

	if (lmvs_fast(timer_node)) {
		lmvs_lfhash_delete(timer->wheels_tbl[*which_wheel], timerid);
		*which_wheel = timer->curr_wheel;
		lmvs_lfhash_insert(timer->wheels_tbl[*which_wheel], timerid, timer_node);
	}
}

void
lmvs_timer_remove(lmvs_timer_t* timer, unsigned int timerid) {
	int* which_wheel = lmvs_lfhash_search(timer->which_wheel_tbl, timerid);
	if (lmvs_slow(!which_wheel)) {
		return;
	}

	int index = *which_wheel;
	free(which_wheel);

	lmvs_timer_node_t* timer_node = lmvs_lfhash_search(timer->wheels_tbl[index], timerid);
	if (timer_node) {
		lmvs_lfhash_delete(timer->wheels_tbl[index], timerid);
		lmvs_lfhash_delete(timer->which_wheel_tbl, timerid);
		free(timer_node);
	}
}

/* Return -1 means there is no expired timer in the wheel,
 * other value means there is expired timers in this wheel */
int
lmvs_timer_book_keeping(lmvs_timer_t* timer) {
	unsigned long now = lmvs_gettime();
	if (now - timer->curr_time < 1000000) {
		return -1;
	}

	timer->curr_time = now;
	timer->prev_wheel = timer->curr_wheel;

	if (++timer->curr_wheel > timer->interval) {
		timer->curr_wheel = 0;
	}

	if (lmvs_lfhash_count(timer->wheels_tbl[timer->curr_wheel]) <= 0) {
		return -1;
	}

	return timer->curr_wheel;
}

