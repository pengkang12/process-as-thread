// -*- C++ -*-
/*
Threads in the same process share:
	- Process instructions
	- most data
	- open files
	- signals and signal handlers
	- current working directory
	- User and group id
Each thread has a unique:
	- thread ID
	- set of register, stack pointer
	- stack for local variables, return addresses
	- signal mask
	- priority
	- Return value: errno
*/
#ifndef MYTHREAD_H_
#define MYTHREAD_H_

#include <stdlib.h>
#include <sys/mman.h>
#include <new>

extern "C" {
	typedef void *threadFunction(void *);
}

// @class ThreadStatus
// @brief Hold the thread id and return value

class mythread {

private:
	class ThreadStatus {
	public:
		ThreadStatus(void * ret, int f) : retval(ret), forked(f) {}
		ThreadStatus(void){}
		// thread's pid
		int tid;
		// thread index in a whole application.
		int threadIndex;
		//return value
		void * retval;
		// I don't know
		bool forked;
	};

	static unsigned int _nestingLevel;
	
	static int _tpid;	

public:
	static void * spawn(threadFunction *fn, void * arg, int parent_index);
	
	static void join(void *threadId, void ** result);
	
	static int cancel(void *threadId);
	
	static int thread_kill(void *threadId, int signal);
	
	static int getThreadIndex(void *threadStatus);
	
	static int getThreadPid(void *threadStatus);
	
	static inline int getId(void){
		return _tpid;
	} 
	
	static inline void setId(int id){
		_tpid = id;
	}

private:
	static void * forkSpawn(threadFunction * fn, ThreadStatus * t, void * arg, int parent_index);

	static void run_thread(threadFunction *fn, ThreadStatus *t, void * arg);

	// shared memory
	static void * allocateSharedObject(size_t sz){
		return mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	}
	
	static void freeSharedObject(void *ptr, size_t sz){
		munmap(ptr, sz);
	}	
};

#endif
