#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define LL_ERROR   0
#define LL_WARNING 1
#define LL_INFO    2
#define LL_DEBUG   3

#define DEBUG(fmt, args ...) do { \
	fprintf(stderr, "[%s:%u:%s()] %ld " fmt "\n", __FILE__, __LINE__, __FUNCTION__, syscall(__NR_gettid), ##args); \
	fflush(stderr); \
} while (0)

#define LOG_DEBUG(log, args...) do { \
        lmvs_log_log(log, LL_DEBUG, __FILE__, __LINE__, args); \
} while (0)

#define LOG_INFO(log, args...) do { \
        lmvs_log_log(log, LL_INFO, __FILE__, __LINE__, args); \
} while (0)

#define LOG_WARNING(log, args...) do { \
        lmvs_log_log(log, LL_WARNING, __FILE__, __LINE__, args); \
} while (0)

#define LOG_ERROR(log, args...) do { \
        lmvs_log_log(log, LL_ERROR, __FILE__, __LINE__, args); \
} while (0)

#define LOG_FATAL(log, args...) do { \
	lmvs_log_fatal(log, __FILE__, __LINE__, args); \
} while (0)

typedef struct lmvs_log lmvs_log_t;

lmvs_log_t* lmvs_log_new(const char* logname, int level, long rotate_size);
void lmvs_log_log(lmvs_log_t*, int level, const char* filename, int lineno, const char* fmt, ...);
void lmvs_log_fatal(lmvs_log_t*, const char* filename, int lineno, const char* fmt, ...);
void lmvs_log_free(lmvs_log_t*);

