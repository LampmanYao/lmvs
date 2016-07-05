#pragma once

#include <pthread.h>

char* lmvs_trim(char* name);
void lmvs_oom(unsigned int size);
int lmvs_cpu_cores();
int lmvs_bind_to_cpu(pthread_t tid, int cpuid);
int lmvs_bound_cpuid(pthread_t tid);
unsigned long lmvs_gettime();
void http_date(char* buffer, size_t len);

