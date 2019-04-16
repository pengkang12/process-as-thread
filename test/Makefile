LIBS = -lm -lrt -ldl
CFLAGS = -Wall -g -O0 # -T../src/dthreads.ld
DTHREAD_LIBS = $(LIBS) -rdynamic ../processes-as-threads/libmythreads.so
PTHREAD_LIBS = $(LIBS) -lpthread

all:
	gcc $(CFLAGS) bug-atom.c      -o bug-atom $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-atom.c      -o bug-atom-myt $(DTHREAD_LIBS)
	gcc $(CFLAGS) bug-deadlock1.c -o bug-deadlock1 $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-deadlock1.c -o bug-deadlock1-myt $(DTHREAD_LIBS)
	gcc $(CFLAGS) bug-deadlock.c  -o bug-deadlock $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-deadlock.c  -o bug-deadlock-myt $(DTHREAD_LIBS)
	gcc $(CFLAGS) bug-lockuse.c   -o bug-lockuse $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-lockuse.c   -o bug-lockuse-myt $(DTHREAD_LIBS)
	gcc $(CFLAGS) bug-order.c     -o bug-order $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-order.c     -o bug-order-myt $(DTHREAD_LIBS)
	gcc $(CFLAGS) bug-race.c      -o bug-race $(PTHREAD_LIBS)
	gcc $(CFLAGS) bug-race.c      -o bug-race-myt $(DTHREAD_LIBS)
	g++ $(CFLAGS) -o lock-mythread lockowner.c $(DTHREAD_LIBS) 
	g++ $(CFLAGS) -o lock-pthread lockowner.c $(PTHREAD_LIBS) 
	g++ $(CFLAGS) -o bug-init-myt bug-init-32bit.c $(DTHREAD_LIBS) 
	g++ $(CFLAGS) -o bug-init bug-init-32bit.c $(PTHREAD_LIBS) 


clean:
	rm -f dthread* Heap*
	rm bug-atom bug-atom-myt
	rm bug-deadlock1 bug-deadlock1-myt
	rm bug-deadlock bug-deadlock-myt
	rm bug-lockuse bug-lockuse-myt
	rm bug-order bug-order-myt
	rm bug-race bug-race-myt
	rm lock-pthread lock-mythread
	rm bug-init bug-init-myt
