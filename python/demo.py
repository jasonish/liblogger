import logger

def log_callback(msg):
    print msg

def main():

    # Initialize the logger.
    logger.init(logger.DEBUG)

    # Add a callback handler.
    h = logger.add_callback_handler(log_callback)
    print h

    # Log an info message.
    logger.info("Testing 1 2 3")

if __name__ == "__main__":
    main()
    
