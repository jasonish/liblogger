#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels. */
enum {
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
};

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
log_handler_t *logger_add_callback_handler(void (*cb)(char *msg, void *arg),
    void *arg);

/**
 * Add a file handler to the logger.  This logs to a file either
 * overwriting an existing file of this name or appending to it.
 *
 * @param filename The filename to log to.
 * @param If 1 the file will be opened in append mode, otherwise the file
 *     will be overwritten if it exists.
 */
log_handler_t *
logger_add_file_handler(char *filename, int append);

/**
 * Add a file pointer log handler.  This will log to an already open
 * file pointer.  Useful for logging to stderr or stdout.
 */
log_handler_t *logger_add_fp_handler(FILE *fp);

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
log_handler_t *logger_add_rotating_handler(const char *filename, int size,
    int count);

/**
 * Initialize the logger module.  This must be called before any other
 * logger functions.  It can be recalled to change the log level.
 *
 * @param log_level The log level to set the logger module to.
 */
void logger_init(int log_level);

/**
 * Free a log handler.  Useful if you want to remove a previously
 * added log handler.
 */
void logger_free_handler(log_handler_t *handler);

/**
 * Reset the logger.  Removes (and frees) all currently active handlers.
 */
void logger_reset(void);

/**
 * Log an information message.
 */
void log_info(const char *fmt, ...);

/**
 * Log a debug message.
 */
void log_debug(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* ! LOGGER_H */
