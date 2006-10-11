/*
 * Copyright (c) 2006 Jason Ish
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* For vasprintf(3) on Linux. */
#define _GNU_SOURCE

#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "queue.h"

#include "logger.h"

/**
 * Logger types.
 */
enum {
	HANDLER_TYPE_NONE = 0,
	HANDLER_TYPE_CALLBACK,
	HANDLER_TYPE_FP,
	HANDLER_TYPE_FILE,
	HANDLER_TYPE_ROTATE,
};

struct log_handler {
	int type;

	void (*output)(log_handler_t *handler, const char *date,
	    const char *msg);

	/* Context information for the different loggers. */
	union {
		/* Callback handler info. */
		struct {
			void (*cb)(char *msg, void *arg);
			void *arg;
		} cb_info;

		/* File pointer and file handler info. */
		struct {
			FILE *fp;
		} fp_info;

		/* Rotating log file handler info. */
		struct rotating_info {
			char filename[FILENAME_MAX];
			FILE *fp;
			int maxsize;
			int count;
			int cursize;
		} rotating_info;
	} u;

	/* Log handlers are kept in a list. */
	TAILQ_ENTRY(log_handler) entries;
};

struct logger {
	int initialized;
	int log_level;
	TAILQ_HEAD(, log_handler) handlers;
};
static struct logger logger;

static void cb_log_handler_output(log_handler_t *handler, const char *date,
    const char *msg);
static void fp_log_handler_output(log_handler_t *handler, const char *date,
    const char *msg);
static void rotating_log_handler_output(log_handler_t *handler,
    const char *date, const char *msg);
static void logger_add_handler(log_handler_t *handler);

log_handler_t *
logger_add_callback_handler(void (*cb)(char *msg, void *arg), void *arg)
{
	log_handler_t *handler;

	handler = calloc(1, sizeof(*handler));
	if (handler) {
		handler->type = HANDLER_TYPE_CALLBACK;
		handler->output = cb_log_handler_output;
		handler->u.cb_info.cb = cb;
		handler->u.cb_info.arg = arg;

		logger_add_handler(handler);
	}

	return handler;
}

static void
cb_log_handler_output(log_handler_t *handler, const char *date,
    const char *msg)
{
	char *fullmsg;

	asprintf(&fullmsg, "%s: %s", date, msg);
	(*handler->u.cb_info.cb)(fullmsg, handler->u.cb_info.arg);
	free(fullmsg);
}

log_handler_t *
logger_add_fp_handler(FILE *fp)
{
	log_handler_t *handler;

	handler = calloc(1, sizeof(*handler));
	if (handler) {
		handler->type = HANDLER_TYPE_FP;
		handler->output = fp_log_handler_output;
		handler->u.fp_info.fp = fp;

		logger_add_handler(handler);
	}
	
	return handler;
}

static void
fp_log_handler_output(log_handler_t *handler, const char *date,
    const char *msg)
{
	fprintf(handler->u.fp_info.fp, "%s: %s\n", date, msg);
}

log_handler_t *
logger_add_file_handler(char *filename, int append)
{
	FILE *fp;
	log_handler_t *handler;

	if (append)
		fp = fopen(filename, "a");
	else
		fp = fopen(filename, "w");
	if (fp == NULL)
		return NULL;

	handler = calloc(1, sizeof(*handler));
	if (handler) {
		handler->type = HANDLER_TYPE_FILE;

		/* We can use the fp handler's output function. and
		 * private data. */
		handler->output = fp_log_handler_output;
		handler->u.fp_info.fp = fp;

		logger_add_handler(handler);
	}
	else
		fclose(fp);

	return handler;
}

log_handler_t *
logger_add_rotating_handler(const char *filename, int size, int count)
{
	log_handler_t *handler;
	FILE *fp;
	struct rotating_info *ri;
	struct stat stat;

	/* First open the file to make sure we can. */
	fp = fopen(filename, "a");
	if (fp == NULL)
		return NULL;

	handler = calloc(1, sizeof(*handler));
	if (handler == NULL)
		return NULL;
	handler->type = HANDLER_TYPE_ROTATE;
	handler->output = rotating_log_handler_output;
	ri = &handler->u.rotating_info;
	ri->fp = fp;
	ri->maxsize = size;
	ri->count = count;
	strncpy(ri->filename, filename, FILENAME_MAX);

	/* The file may have existed already, get the current size. */
	fstat(fileno(fp), &stat);
	ri->cursize = stat.st_size;

	logger_add_handler(handler);

	return handler;
}

static void
rotating_log_handler_rotate(struct rotating_info *ri)
{
	char oldpath[FILENAME_MAX], newpath[FILENAME_MAX];
	int i;

	/* Close the current log file. */
	fclose(ri->fp);

	for (i = ri->count; i > 0; i--) {
		snprintf(oldpath, FILENAME_MAX, "%s.%d", ri->filename, i);
		snprintf(newpath, FILENAME_MAX, "%s.%d", ri->filename, i + 1);
		rename(oldpath, newpath);
	}
	snprintf(newpath, FILENAME_MAX, "%s.1", ri->filename);
	rename(ri->filename, newpath);

	ri->fp = fopen(ri->filename, "w");
	ri->cursize = 0;
}

static void
rotating_log_handler_output(log_handler_t *handler, const char *date,
    const char *msg)
{
	struct rotating_info *ri = &handler->u.rotating_info;

	ri->cursize += fprintf(ri->fp, "%s: %s\n", date, msg);
	if ((ri->maxsize > 0) && (ri->cursize > ri->maxsize))
		rotating_log_handler_rotate(ri);
}

void
logger_init(int log_level)
{
	if (!logger.initialized) {
		TAILQ_INIT(&logger.handlers);
		logger.initialized = 1;
	}
	logger.log_level = log_level;
}

static void
logger_add_handler(log_handler_t *handler)
{
	assert(logger.initialized);

	TAILQ_INSERT_TAIL(&logger.handlers, handler, entries);
}

static void
logger_log(int log_level, const char *fmt, va_list ap)
{
	char date[256]; /* Plenty of space for a date. */
	char *msg;
	log_handler_t *handler;
	struct timeval tv;
	struct tm tm;

	if (!logger.initialized)
		return;

	if (log_level >= logger.log_level) {
		/* Build the time. */
		gettimeofday(&tv, NULL);
		localtime_r((time_t *)&tv.tv_sec, &tm);
		snprintf(date, sizeof(date),
		    "%d-%02d-%02d %02d:%02d:%02d,%03ld",
		    1900 + tm.tm_year,
		    tm.tm_mon + 1,
		    tm.tm_mday,
		    tm.tm_hour,
		    tm.tm_min,
		    tm.tm_sec,
		    tv.tv_usec / 1000);

		vasprintf(&msg, fmt, ap);

		TAILQ_FOREACH(handler, &logger.handlers, entries) {
			(*handler->output)(handler, date, msg);
		}

		free(msg);
	}
}

void
log_info(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	logger_log(LOG_LEVEL_INFO, fmt, ap);
	va_end(ap);
}

void
log_debug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	logger_log(LOG_LEVEL_INFO, fmt, ap);
	va_end(ap);
}

void
logger_reset(void)
{
	log_handler_t *handler;
	
	/* Remove all log handler. */
	while ((handler = TAILQ_FIRST(&logger.handlers))) {
		logger_remove_handler(handler);
	}
}
	    
void
logger_remove_handler(log_handler_t *handler)
{
	/* If the logger is initialized, remove this specific handler
	 * from the handler list. */
	if (logger.initialized) {
		log_handler_t *h, *h0;

		for (h = TAILQ_FIRST(&logger.handlers); h != NULL; h = h0) {
			h0 = TAILQ_NEXT(h, entries);
			if (h == handler)
				TAILQ_REMOVE(&logger.handlers, h, entries);
		}
	}

	switch (handler->type) {
	case HANDLER_TYPE_FILE:
		/* Close the open file. */
		fclose(handler->u.fp_info.fp);
	case HANDLER_TYPE_ROTATE:
		/* Close the currently open file. */
		fclose(handler->u.rotating_info.fp);
		break;
	default:
		break;
	}

	/* Finally free the handler. */
	free(handler);
}
