#ifndef MYRUN_H_
#define MYRUN_H_

#include <stdio.h>
#include "xdefines.h"
#include "atomic.h"
#include "mythread.h"
#include "mydeterm.h"
#include "xmemory.h"

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

public: 
	static inline void initialize(){}
	static inline void * spawn(threadFunction * fn, void * arg){
		
		
		//child thread number increment.
		_children_threads_count++;
		
		void * ptr = mythread::spawn(fn, arg, _thread_index);
		
		return ptr;
	}
	static inline int waitChildRegister(void){
		printf("waitChildRegistered\n");	
		mydeterm::getInstance().waitChildRegister();
	}
	// New created threads are waiting until notify by main thread.
  	static void notifyWaitingParent(void) {
    	mydeterm::getInstance().notifyWaitingParent();
  	}
	static inline int childRegister(int pid, int parentIndex){
//		printf("childRegistered\n");	
		int threads;
		//assign a global thread index for child thread.
		_thread_index = atomic_increment_and_return(&global_data->thread_index);
		_lock_count = 0;	
 
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
		bool wakeupChildren = false;
	
		child_threadindex = mythread::getThreadIndex(v);
		mythread::join(v, result);
	
	}
	static inline int mutex_init(pthread_mutex_t *mutex){
		mydeterm::getInstance().lock_init((void *)mutex);
		return 0;
	}
	
	static inline void mutex_lock(pthread_mutex_t *mutex){
		_lock_count++;
		bool getLock;
		do{	
			atomicBegin(true);
			getLock = mydeterm::getInstance().lock_acquire(mutex);
		}while(getLock == false);
	}
	
	static inline void mutex_unlock(pthread_mutex_t *mutex){
		_lock_count--;
		mydeterm::getInstance().lock_release(mutex);
		atomicEnd(false);
	}
	
	static inline int mutex_destroy(pthread_mutex_t *mutex){
		mydeterm::getInstance().lock_destroy(mutex);	
		return 0;
	}
	
	static void cond_init(void *cond){
		mydeterm::getInstance().cond_init(cond);
	}
	static void cond_destroy(void *cond){
		mydeterm::getInstance().cond_destroy(cond);
	}
	static void cond_wait(void *cond, void *lock){

	}

	static void cond_broadcast(void *cond){
	}
	static void cond_signal(void *cond){
	}

	static int barrier_init(pthread_barrier_t *barrier, unsigned int count){
		mydeterm::getInstance().barrier_init(barrier, count);
	}
	
	static int barrier_destroy(pthread_barrier_t *barrier){
		mydeterm::getInstance().barrier_destroy(barrier);	
	}
	
	static int barrier_wait(pthread_barrier_t *barrier) {
		
	}

	static void atomicBegin(bool cleanup){
		fflush(stdout);
		xmemory::begin(cleanup);	
	}
	static void atomicEnd(bool update){
		fflush(stdout);
		xmemory::commit(update);
	}
	
};

#endif
