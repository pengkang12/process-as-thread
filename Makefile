LIBS = -lm -lrt -ldl
CFLAGS = -Wall -g -O3 # -T../src/dthreads.ld
DTHREAD_LIBS = $(LIBS) -rdynamic  processes-as-threads/libdthread.so #~/project/dthreads/src/libdthread.so
PTHREAD_LIBS = $(LIBS) -lpthread

all:
	gcc $(CFLAGS) first.c -o first $(DTHREAD_LIBS)
