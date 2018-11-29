LIBS = -lm -lrt -ldl
CFLAGS = -Wall -g -O3 -std=c++0x # -T../src/dthreads.ld
DTHREAD_LIBS = $(LIBS) #-rdynamic  processes-as-threads/libdthread.so #~/project/dthreads/src/libdthread.so
PTHREAD_LIBS = $(LIBS) -lpthread

all:
	gcc $(CFLAGS) first.cpp -o first $(DTHREAD_LIBS)
