#include <pthread.h>
#include <syscall.h>
#include <stdio.h>

#include "myrun.h"

runtime_data_t * global_data;
static bool initialized = false;

void finalize() __attribute__((destructor)); 
__attribute__((constructor)) void initialize() {

	init_real_functions();
	
	global_data = (runtime_data_t *)mmap(NULL, xdefines::PageSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	//parent thread index should be 1.
	global_data->thread_index = 1;
	myrun::initialize();
	
	initialized = true;
	
	printf("this is my first time print\n");
}

void finalize(){
	//printf("this is my end print\n");
}

int pthread_create (pthread_t * tid, const pthread_attr_t * attr, void *(*fn) (void *), void * arg) {
	//assert(initialized);
	if(initialized) {
		*tid = (pthread_t)myrun::spawn(fn, arg);	
	}
	printf("haha, this is my first step\n");
	return 0;
}

int pthread_join(pthread_t tid, void ** val) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}


void pthread_exit(void * value_ptr) {
	if(initialized) {
		myrun::deleteChildRegister();
	}
	_exit(0);	
}

int pthread_cancel(pthread_t thread) {
	if(initialized) {
		myrun::cancel((void*) thread);
	}
	return 0;
}

int pthread_setconcurrency(int) {
	return 0;
}

int pthread_attr_init(pthread_attr_t *) {
	return 0;
}

int pthread_attr_destroy(pthread_attr_t *) {
	return 0;
}

pthread_t pthread_self(void) {
	if(initialized) {
	}
	return 0;
}

int pthread_kill(pthread_t thread, int sig) {
	return 0;
}


int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *) {
	if(initialized) {
		return myrun::mutex_init(mutex);
	}
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
	if(initialized) {
		myrun::mutex_lock(mutex);
	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	if(initialized) {
		myrun::mutex_unlock(mutex);
	}
	return 0;
}

int pthread_mutex_destory(pthread_mutex_t *mutex) {
	if(initialized) {
		return myrun::mutex_destroy(mutex);
	}
	return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *, size_t * s) {
	*s = 1048576UL; // really? FIX ME
	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *) {
	return 0;
}
int pthread_mutexattr_init(pthread_mutexattr_t *) {
	return 0;
}
int pthread_mutexattr_settype(pthread_mutexattr_t *, int) {
	return 0;
}
int pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *) {
	return 0;
}
int pthread_attr_setstacksize(pthread_attr_t *, size_t) {
	return 0;
}

int pthread_cond_init(pthread_cond_t * cond, const pthread_condattr_t *attr) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_cond_broadcast(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_cond_signal(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_cond_destroy(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

// Add support for barrier functions
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t * attr, unsigned int count) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
	//assert(initialized);
	if(initialized) {
	}
	return 0;
}