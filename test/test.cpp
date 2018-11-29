#include <stdio.h>
#include <pthread.h>

//#include "mythread.h"

void initialize(){
	printf("start\n");
}
void finalize(){
	printf("end\n");	
}

int main(void){
	//printf("sizeof xheap is %d\n", sizeof(xheap));	
	printf("hello world\n");
	//test();	


	return 0;
}
