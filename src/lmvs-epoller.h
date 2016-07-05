#pragma once

#include <stdbool.h>
#include <sys/epoll.h>

typedef struct epoll_event lmvs_epoller_event_t;

typedef struct lmvs_epoller {
	int fd;
	int max_events;
	lmvs_epoller_event_t* events;
} lmvs_epoller_t;

lmvs_epoller_t* lmvs_epoller_new(int max_events);
void lmvs_epoller_free(lmvs_epoller_t*);

int lmvs_epoller_add(lmvs_epoller_t*, int fd, unsigned int sid);
int lmvs_epoller_del(lmvs_epoller_t*, int fd, unsigned int sid);
int lmvs_epoller_mod_read(lmvs_epoller_t*, int fd, unsigned int sid);
int lmvs_epoller_mod_write(lmvs_epoller_t*, int fd, unsigned int sid);
int lmvs_epoller_mod_rw(lmvs_epoller_t*, int fd, unsigned int sid);

int lmvs_epoller_wait(lmvs_epoller_t*, int timeout /* milliseconds */);
lmvs_epoller_event_t* lmvs_epoller_get_event(lmvs_epoller_t*, int index);
bool lmvs_epoller_event_is_readable(lmvs_epoller_event_t* event);
bool lmvs_epoller_event_is_writeable(lmvs_epoller_event_t* event);
bool lmvs_epoller_event_is_error(lmvs_epoller_event_t* event);
int lmvs_epoller_event_fd(lmvs_epoller_event_t* event);
unsigned int lmvs_epoller_event_sid(lmvs_epoller_event_t* event);

