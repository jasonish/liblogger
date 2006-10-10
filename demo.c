#include "logger.h"

void
log_callback(char *msg, void *arg)
{
	printf("Received in callback: %s\n", msg);
}

int
main(int argc, char **argv)
{
	/* Initialize the logger module. */
	logger_init(LOG_LEVEL_DEBUG);

	/* Log to stderr. */
	logger_add_fp_handler(stderr);

	/* Add a callback handler. */
	logger_add_callback_handler(log_callback, NULL);

	/* Log an info message. */
	log_info("This is an info test: %s:%d", __FILE__, __LINE__);

	/* Reset the logger - frees all allocated resources and keeps
	 * valgrind happy. */
	logger_reset();

	return 0;
}

