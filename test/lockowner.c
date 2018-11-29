#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "time.h" 
#define CORES 2

pthread_mutex_t mutex[CORES];
int number = 10;
const int b = 1;
/* 1ms work in different machine. */
void unit_work(void)
{
  volatile int i;
  volatile int f,f1,f2;

	f1 =12441331;
	f2 = 3245235;
	for (i = 47880000; i > 0; i--) {
                f *= f1;        /*    x    x     1/x */
                f1 *= f2;       /*    x    1     1/x */
                f1 *= f2;       /*    x    1/x   1/x */
                f2 *= f;        /*    x    1/x   1   */
                f2 *= f;        /*    x    1/x   x   */
                f *= f1;        /*    1    1/x   x   */
                f *= f1;        /*    1/x  1/x   x   */
                f1 *= f2;       /*    1/x  1     x   */
                f1 *= f2;       /*    1/x  x     x   */
                f2 *= f;        /*    1/x  x     1   */
                f2 *= f;        /*    1/x  x     1/x */
                f *= f1;        /*    1    x     1/x */
        }
}

void * child_thread(void * data)
{
	unsigned long threadid = (unsigned long)data;
	int rounds = 3;	
	int i;
	int j;
	number = number +1;
    /* Do specified work. */	
	for(i = 0; i < rounds; i++)
	{
		pthread_mutex_lock(&mutex[threadid]);
		number = number + 1;
		fprintf (stderr, "threadid:%d, number is %d\n", threadid, number);
		fflush (stderr);
	
		/* Do 1ms computation work. */
		for(j = 0; j < 1; j++) {
	//		fprintf(stderr, "threadid:%d with j %d\n", threadid, j);
			unit_work();
		}
	
		pthread_mutex_unlock(&mutex[threadid]);
	}
	number = number+1;
	sleep(20);
	return NULL;
} 


/* Different input parameters
 */
int main(int argc,char**argv)
{
	int i;
	pthread_t threads[CORES];

	//printf("initialize the control informaiton\n");
	/* Initialize common control information. */
	for(int i = 0; i < CORES; i++)
	{
		pthread_mutex_init(&mutex[i], NULL);
	}
	int fd = mkstemp("dthreadsMxxxx");
	void * addr1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, fd, 0);
	for(int i = 0; i < CORES; i++)
		pthread_create (&threads[i], NULL, child_thread, (void *)i);
	for(i = 0; i < CORES; i++) {
		pthread_join (threads[i], NULL);
	}
	sleep(50);
	printf("number is %d, b is %lx, addr is %lx \n", number, &b, (long unsigned int)addr1);
	
	return 0;
}
