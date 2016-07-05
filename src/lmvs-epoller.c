#include "lmvs-epoller.h"
#include "lmvs-log.h"
#include "lmvs-utils.h"

#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

lmvs_epoller_t*
lmvs_epoller_new(int max_events) {
	lmvs_epoller_t* epoller = calloc(1, sizeof(*epoller));
	if (!epoller) {
		lmvs_oom(sizeof(*epoller));
	}

	epoller->fd = epoll_create(1024);
	if (epoller->fd == -1) {
		DEBUG("epoll_create(): %s", strerror(errno));
		free(epoller);
		return NULL;
	}

	epoller->max_events = max_events;
	epoller->events = calloc(max_events, sizeof(lmvs_epoller_event_t));

	if (!epoller->events) {
		lmvs_oom(max_events * sizeof(lmvs_epoller_event_t));
	}

	return epoller;
}

void
lmvs_epoller_free(lmvs_epoller_t* epoller) {
	assert(epoller);
	close(epoller->fd);
	free(epoller->events);
	free(epoller);
}

int
lmvs_epoller_add(lmvs_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_ADD, fd, &ev);
}

int
lmvs_epoller_del(lmvs_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, &ev);
}

int
lmvs_epoller_mod_read(lmvs_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
lmvs_epoller_mod_write(lmvs_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLOUT | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
lmvs_epoller_mod_rw(lmvs_epoller_t* epoller, int fd, unsigned int sid) {
	assert(epoller);
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLOUT | EPOLLET,
		.data.u64 = (unsigned long)fd << 32 | sid
	};
	return epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &ev);
}

int
lmvs_epoller_wait(lmvs_epoller_t* epoller, int timeout /* milliseconds */) {
	assert(epoller);
	return epoll_wait(epoller->fd, epoller->events, epoller->max_events, timeout);
}

lmvs_epoller_event_t*
lmvs_epoller_get_event(lmvs_epoller_t* epoller, int index) {
	assert(epoller);
	if (index < epoller->max_events) {
		return &epoller->events[index];
	}
	return NULL;
}

bool
lmvs_epoller_event_is_readable(lmvs_epoller_event_t* event) {
	assert(event);
	return event->events & EPOLLIN;
}

bool
lmvs_epoller_event_is_writeable(lmvs_epoller_event_t* event) {
	assert(event);
	return event->events & EPOLLOUT;
}

bool
lmvs_epoller_event_is_error(lmvs_epoller_event_t* event) {
	assert(event);
	return event->events & (EPOLLERR | EPOLLHUP);
}

int
lmvs_epoller_event_fd(lmvs_epoller_event_t* event) {
	assert(event);
	return event->data.u64 >> 32;
}

unsigned int
lmvs_epoller_event_sid(lmvs_epoller_event_t* event) {
	assert(event);
	return event->data.u64 & 0xffffUL;
}

