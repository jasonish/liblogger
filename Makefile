CFLAGS =	-g -O2 -Wall

SRCS =		logger.c
OBJS =		$(SRCS:.c=.o)

.PHONY:		doc

all: liblogger.a demo

liblogger.a: $(OBJS)
	ar r $@ $(OBJS)

demo: demo.o liblogger.a
	$(CC) $(CFLAGS) -o $@ demo.c -L. -llogger

doc:
	doxygen

clean:
	rm -f *.o *~ *.a
	rm -f demo demo_log
	cd python && $(MAKE) $@
	rm -rf doc
