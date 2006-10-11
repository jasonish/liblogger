INCLUDE = -I/usr/include/python2.3
CFLAGS = -O2 -g $(INCLUDE)

PYREXC ?= pyrexc

logger.so: logger.o
	$(CC) -fPIC -shared -o logger.so logger.o ../liblogger.a

logger.c: logger.pyx
	$(PYREXC) logger.pyx

clean:
	rm -f *.so *.o *~ *.c

