#include "lmvs.h"
#include "lmvs-log.h"
#include "lmvs-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

char* root_path;

int main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s config\n", argv[0]);
		fflush(stderr);
		return -1;
	}

	struct rlimit rlimit;
	rlimit.rlim_cur = 1024 * 1000;
	rlimit.rlim_max = 1024 * 1000;
	if (setrlimit(RLIMIT_NOFILE, &rlimit)) {
		fprintf(stderr, "setrlimit RLIMIT_NOFILE failed\n");
		fflush(stderr);
		return -1;
	}

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGPIPE, SIG_IGN);  /* avoid send() crashes */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/*
	daemon(1, 1);
	*/

	char* logfile;
	char* logsize;
	char* loglevel;
	char* port;
	char* maxconn;
	char* threadcount;
	char* keep_alive;

	lmvs_config_t* config;
	lmvs_log_t* logger;
	lmvs_t* lmvs;

	config = lmvs_config_new();
	lmvs_config_load(config, argv[1]);

	logfile = lmvs_config_find(config, "logfile", strlen("logfile"));
	logsize = lmvs_config_find(config, "logsize", strlen("logsize"));
	loglevel = lmvs_config_find(config, "loglevel", strlen("loglevel"));

	if (!logfile) {
		fprintf(stderr, "could not find `logfile`!");
		fflush(stderr);
		lmvs_config_free(config);
		return -1;
	}

	if (!logsize) {
		fprintf(stderr, "could not find `logsize`!");
		fflush(stderr);
		lmvs_config_free(config);
		return -1;
	}

	if (!loglevel) {
		fprintf(stderr, "could not find `loglevel`!");
		fflush(stderr);
		lmvs_config_free(config);
		return -1;
	}

	logger = lmvs_log_new(logfile, atoi(loglevel), atoi(logsize) * 1024 * 1024);
	if (!logger) {
		fprintf(stderr, "can not open logfile\n");
		fflush(stderr);
		lmvs_config_free(config);
		return -1;
	}

	port = lmvs_config_find(config, "port", strlen("port"));
	maxconn = lmvs_config_find(config, "maxconn", strlen("maxconn"));
	threadcount = lmvs_config_find(config, "threadcount", strlen("threadcount"));
	keep_alive = lmvs_config_find(config, "keep_alive", strlen("keep_alive"));
	root_path = lmvs_config_find(config, "root_path", strlen("root_path"));

	if (!port) {
		LOG_WARNING(logger, "could not find `port`! Use '8080' as default!");
		port = "8080";
	}

	if (!maxconn) {
		LOG_WARNING(logger, "could not find `maxconn`! Use '1000' as default!");
		maxconn = "1000";
	}

	if (!threadcount) {
		LOG_WARNING(logger, "could not find `threadcount`! Use '4' as default!");
	}

	if (!keep_alive) {
		LOG_WARNING(logger, "could not find `keep_alive`! Use '360' as default!");
		keep_alive = "360";
	}

	if (!root_path) {
		LOG_FATAL(logger, "could not find `root_path`!");
	}

	lmvs = lmvs_new(atoi(port), atoi(threadcount), atoi(maxconn), atoi(keep_alive), logger);
	LOG_INFO(logger, "Server start ok ...");
	lmvs_loop(lmvs, -1);

	lmvs_free(lmvs);
	lmvs_config_free(config);
	lmvs_log_free(logger);

        return 0;
}

