#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
//#include "mythread.h"
extern void initializer();

void initialize(){
	printf("start\n");
}
void finalize(){
	printf("end\n");	
}
pthread_mutex_t mutex;

int array[1000];

int main(void){
	//printf("sizeof xheap is %d\n", sizeof(xheap));	
	printf("hello world\n");
	//pthread_mutex_init(&mutex, NULL);	
	sleep(100);
	return 0;
}
