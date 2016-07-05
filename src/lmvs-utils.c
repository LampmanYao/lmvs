#include "lmvs-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>

char*
lmvs_trim(char* name) {
	char* start_pos = name;
	while ((*start_pos == ' ') || (*start_pos == '\t')) {
		start_pos++;
	}
	if (strlen(start_pos) == 0) {
		return NULL; 
	}
	char* end_pos = name + strlen(name) - 1;
	while ((*end_pos == ' ') || (*end_pos == '\t')) {
		*end_pos = 0;
		end_pos--;
	}
	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0) {
		return NULL; 
	}
	return start_pos;
}

void
lmvs_oom(unsigned int size) {
	fprintf(stderr, "out of memory trying to allocate %u bytes\n", size);
	fflush(stderr);
	abort();
}

inline int
lmvs_cpu_cores(void) {
	int cores = sysconf(_SC_NPROCESSORS_ONLN);
	return cores > 0 ? cores : 1;
}

inline int
lmvs_bind_to_cpu(pthread_t tid, int cpuid) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);
	return pthread_setaffinity_np(tid, sizeof(mask), &mask);
}

inline int
lmvs_bound_cpuid(pthread_t tid) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	pthread_getaffinity_np(tid, sizeof(mask), &mask);
	int cpu_cores = lmvs_cpu_cores();
	for (int i = 0; i < cpu_cores; i++) {
		if (CPU_ISSET(i, &mask)) {
			return i;
		}
	}
	return -1;
}

inline unsigned long
lmvs_gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

inline void
http_date(char* buffer, size_t len) {
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(buffer, len, "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
}

