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

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup logger
 *
 * This module implements a basic, simple to use logging facility for
 * your application.
 *
 * @{
 */

/**
 * Available logging levels.
 */
enum log_levels {
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_DEBUG,    /**< Debug. */
	LOG_LEVEL_INFO,     /**< Informational. */
};

/**
 * The opaque datatype to deal with log handlers.
 */
typedef struct log_handler log_handler_t;

/**
 * Add a callback handler to the logger.  This type of handler calls
 * the application supplied callback with the constructed log message
 * and the user supplied argument.
 *
 * @param cb The callback function to call.
 * @param arg A user supplied argument that will be passed to the callback.
 *
 * @return Returns a pointer to the allocated log_handler.
 */
log_handler_t *
logger_add_callback_handler(void (*cb)(char *msg, void *arg), void *arg);

/**
 * Add a file handler to the logger.  This logs to a file either
 * overwriting an existing file of this name or appending to it.
 *
 * @param filename The filename to log to.
 * @param append If 1 the file will be opened in append mode, otherwise
 *     the file will be overwritten if it exists.
 */
log_handler_t *
logger_add_file_handler(char *filename, int append);

/**
 * Add a file pointer log handler.  This will log to an already open
 * file pointer.  Useful for logging to stderr or stdout.
 */
log_handler_t *
logger_add_fp_handler(FILE *fp);

/**
 * Setup a rotating log file handler.  This will log to filename until
 * filename becomes at least size bytes at which time filename will be
 * renamed to filename.1, filename.1 will be renamed to filename.2 and
 * so on.
 *
 * @param filename The filename to log to.
 * @param size The size of each log file.
 * @param count The number of log files to keep around.
 */
log_handler_t *
logger_add_rotating_handler(const char *filename, int size, int count);

/**
 * Remove a handler from the logger.  The pointer to the handler is
 * invalid after this operation as its resources will have been freed.
 *
 * @param handler A pointer to the handler to free.
 */
void
logger_remove_handler(log_handler_t *handler);

/**
 * Initialize the logger module.  This must be called before any other
 * logger functions.  It can be recalled to change the log level.
 *
 * @param log_level The log level to set the logger module to.
 */
void
logger_init(int log_level);

/**
 * Reset the logger.  Removes (and frees) all currently active handlers.
 */
void
logger_reset(void);

/**
 * Log an information message.
 */
void
log_info(const char *fmt, ...);

/**
 * Log a debug message.
 */
void
log_debug(const char *fmt, ...);

/* }@ */

#ifdef __cplusplus
}
#endif

#endif /* ! LOGGER_H */
