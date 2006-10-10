CFLAGS =	-g -O2 -Wall

SRCS =		logger.c
OBJS =		$(SRCS:.c=.o)

all: liblogger.a demo

liblogger.a: $(OBJS)
	ar r $@ $^

demo: demo.o liblogger.a
	$(CC) $(CFLAGS) -o $@ demo.c liblogger.a

clean:
	rm -f *.o *~ *.a
	rm -f demo
	cd python && $(MAKE) $@