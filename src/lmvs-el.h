#pragma once

#include "lmvs-log.h"
#include "lmvs-timer.h"
#include "lmvs-epoller.h"
#include "lmvs-sockset.h"

typedef struct lmvs_el {
	int max_conn;
	int cur_conn;
	lmvs_epoller_t* epoller;
	lmvs_sockset_t* sockset;
	lmvs_timer_t* timer;
	lmvs_log_t* log;
} lmvs_el_t;

lmvs_el_t* lmvs_el_new(int max_conn, int connect_timeout, lmvs_log_t* log);
void lmvs_el_free(lmvs_el_t*);
int lmvs_el_add_connection(lmvs_el_t*, int fd);
void* lmvs_el_io_loop(void* arg);

