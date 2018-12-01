#ifndef MYRUN_H_
#define MYRUN_H_

#include <pthread.h>
#include <stdio.h>
#include "xdefines.h"
#include "atomic.h"
#include "mythread.h"
#include "mydeterm.h"
#include "xmemory.h"
#include "libfuncs.h"

class myrun {
private:
	// we maintain a counter number. To help us differ thread. Thread index start from Zero.
	static size_t _thread_index;
	// thread whether is initialized
	static volatile bool _initialized;

  	//static volatile bool _protection_enabled;
  	//record main thread id.
	static size_t _master_thread_id;
	
  	//static bool _fence_enabled;
  	//How much child thread does the current thread have?
	static size_t _children_threads_count;
  	//How much lock does the current thread have?
	static size_t _lock_count;
  	//static bool _token_holding;		
	static pthread_condattr_t _condattr;
	static pthread_mutexattr_t _mutexattr;
public: 
	static inline void initialize()
	{
		WRAP(pthread_mutexattr_init)(&_mutexattr);
		pthread_mutexattr_setpshared(&_mutexattr, PTHREAD_PROCESS_SHARED);
		WRAP(pthread_condattr_init)(&_condattr);
		pthread_condattr_setpshared(&_condattr, PTHREAD_PROCESS_SHARED);
	}
	static inline void * spawn(threadFunction * fn, void * arg){
		
		
		//child thread number increment.
		_children_threads_count++;
		
		void * ptr = mythread::spawn(fn, arg, _thread_index);
		
		return ptr;
	}
	
	static inline int childRegister(int pid, int parentIndex){
		int threads;
		//assign a global thread index for child thread.
		_thread_index = atomic_increment_and_return(&global_data->thread_index);
		_lock_count = 0;	
		 
		printf("childRegistered, index %d\n", _thread_index);	
		mydeterm::getInstance().registerThread(_thread_index, pid, parentIndex);
	}	
	static inline void deleteChildRegister(void){
		//printf("deleteChildRegistered information\n");
    	mydeterm::getInstance().deleteRegisterThread(_thread_index);
	}
	static inline void cancel(void *thread){
	}	
	
	static inline void join(void *v, void ** result){
		int child_threadindex = 0;
		
		child_threadindex = mythread::getThreadIndex(v);
		mydeterm::getInstance().join(child_threadindex, _thread_index);
	}

	static inline int mutex_init(pthread_mutex_t *mutex){
		WRAP(pthread_mutex_init)(mutex, &_mutexattr);	
		
		//mydeterm::getInstance().lock_init((void *)mutex);
		return 0;
	}
	
	static inline void mutex_lock(pthread_mutex_t *mutex){
		
		_lock_count++;
		WRAP(pthread_mutex_lock)(mutex);
	}
	
	static inline void mutex_unlock(pthread_mutex_t *mutex){
		_lock_count--;
		WRAP(pthread_mutex_unlock)(mutex);
	}
	
	static inline int mutex_destroy(pthread_mutex_t *mutex){
		WRAP(pthread_mutex_destroy)(mutex);
		//mydeterm::getInstance().lock_destroy(mutex);	
		return 0;
	}
	static void waitParentNotify(void) {
    	mydeterm::getInstance().waitParentNotify();
  	}

  	static inline void waitChildRegistered(void) {
   		mydeterm::getInstance().waitChildRegistered();
  	}		
	static void cond_init(pthread_cond_t *cond){
		WRAP(pthread_cond_init)(cond, &_condattr);	
	}
	static void cond_destroy(pthread_cond_t *cond){
		WRAP(pthread_cond_destroy)(cond);	
	}
	static void cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock){
		WRAP(pthread_cond_wait)(cond, lock);	
	}

	static void cond_broadcast(pthread_cond_t *cond){
		WRAP(pthread_cond_broadcast)(cond);	
	}
	static void cond_signal(pthread_cond_t *cond){
		WRAP(pthread_cond_signal)(cond);	
	}

	static int barrier_init(pthread_barrier_t *barrier, unsigned int count){
		WRAP(pthread_barrier_init)(barrier, NULL, count);	
	}

	static int barrier_destroy(pthread_barrier_t *barrier){
		WRAP(pthread_barrier_destroy)(barrier);	
	}
	
	static int barrier_wait(pthread_barrier_t *barrier) {
		WRAP(pthread_barrier_destroy)(barrier);		
	}

};

#endif
