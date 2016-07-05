#pragma once

#include "lmvs-el.h"
#include "lmvs-log.h"
#include "lmvs-epoller.h"

typedef struct lmvs {
	int listenfd;
	int cpu_cores;
	int thread_count;
	int max_conn;
	lmvs_epoller_t* epoller;
	lmvs_log_t* log;
	lmvs_el_t* el_list[0];
} lmvs_t;

lmvs_t* lmvs_new(int port, int thread_count, int max_conn, int connect_timeout, lmvs_log_t* log);
void lmvs_free(lmvs_t*);
void lmvs_loop(lmvs_t*, int timeout);

