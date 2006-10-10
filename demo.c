#include "logger.h"

void
log_callback(char *msg, void *arg)
{
	printf("Received in callback: %s\n", msg);
}

int
main(int argc, char **argv)
{
	int i;

	/* Initialize the logger module. */
	logger_init(LOG_LEVEL_DEBUG);

	/* Log to stderr. */
	logger_add_fp_handler(stderr);

	/* Add a callback handler. */
	logger_add_callback_handler(log_callback, NULL);

	/* Add a rotating file handler. */
	logger_add_rotating_handler("demo_log", 5000, 3);

	/* Log an info message. */
	for (i = 0; i < 5; i++) {
		log_info("Info log %d", i);
		log_debug("Debug log %d", i);
	}

	/* Reset the logger - frees all allocated resources and keeps
	 * valgrind happy. */
	logger_reset();

	return 0;
}

