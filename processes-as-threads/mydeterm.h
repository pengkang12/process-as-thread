#ifndef MYTHREADS_DETERM_H__
#define MYTHREADS_DETERM_H__
#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "list.h"
#include "internalheap.h"
#include "xmemory.h"

#define MAX_THREADS 1024


class mydeterm {
private:
   
  	// Different status of one thread.
  	enum threadStatus {
    	STATUS_COND_WAITING = 0, STATUS_BARR_WAITING, STATUS_READY, STATUS_EXIT, STATUS_JOINING
  	};
	
	// Each thread has a thread entry in the system, it is used to control the thread. 
  	// For example, when one thread is cond_wait, the corresponding thread entry will be taken out of the token
 	// queue and putted into corresponding conditional variable queue.
  	class ThreadEntry {
  	public:
    	inline ThreadEntry():prev(NULL), next(NULL){}

    	inline ThreadEntry(int tid, int threadindex) {
      		this->tid = tid;
      		this->threadindex = threadindex;
      		this->wait = 0;
			this->prev = NULL;
			this->next = NULL;
    	}	

    	list_t * prev;
    	list_t * next;
    	volatile int tid; // pid of this thread.
    	volatile int threadindex; // thread index 
    	volatile int status;
    	int tid_parent; // parent's pid
    	void * cond; 
    	void * barrier;
    	size_t wait;
    	int joinee_thread_index;
  	};
	
	pthread_mutex_t _mutex;
	pthread_cond_t cond;
	pthread_condattr_t _condattr;
	pthread_mutexattr_t _mutexattr;
	
	// When one thread is created, it will wait until all threads are created.
  	// The following two flag are used to indentify whether one thread can move on or not.
  	volatile bool _childregistered;
  	volatile bool _parentnotified;

	pthread_cond_t _cond_children;
	pthread_cond_t _cond_parent;
	pthread_cond_t _cond_join;

	// All threads should be putted into this active list.
	// Here, header is not one node in the circular link list.
	// _activelist->next is the first node in the link list, while _activelist->prev is
	// the last node in the link list.
 	list_t *_activeList;
	ThreadEntry _entries[MAX_THREADS];

	size_t _coreNumber;	
	//how many do thread entries have in system. 
	size_t _maxthreadentries;

	size_t _condnum;
	size_t _barriernum;
	
 	// Variables related to token pass and fence control
  	volatile ThreadEntry *_tokenpos;
  	
	//thread statistic information.
	volatile size_t _maxthreads;
  	volatile size_t _currthreads;
  	volatile bool _is_arrival_phase;
  	volatile size_t _alivethreads;
	
	mydeterm():
    _condnum(0),
    _barriernum(0),
    _maxthreads(0),
    _currthreads(0),
    //_is_arrival_phase(false),
    _alivethreads(0),
    _maxthreadentries(MAX_THREADS),
    _activeList(NULL),
    //_tokenpos(NULL), 
   	_parentnotified(false), 
    _childregistered(false) 
    {  }

public:
	void initialize(void){
		_coreNumber = sysconf(_SC_NPROCESSORS_ONLN);
		if(_coreNumber < 1) {
			//Does this situation exist?
      		fprintf(stderr, "cores number is not correct. Exit now.\n");
      		exit(-1);
    	}
		
		WRAP(pthread_mutexattr_init)(&_mutexattr);
		pthread_mutexattr_setpshared(&_mutexattr, PTHREAD_PROCESS_SHARED);
		WRAP(pthread_condattr_init)(&_condattr);
		pthread_condattr_setpshared(&_condattr, PTHREAD_PROCESS_SHARED);	
	
		WRAP(pthread_mutex_init)(&_mutex, &_mutexattr) ;
		WRAP(pthread_cond_init)(&cond, &_condattr);
		WRAP(pthread_cond_init)(&_cond_parent, &_condattr);
	
		WRAP(pthread_cond_init)(&_cond_children, &_condattr);
		WRAP(pthread_cond_init)(&_cond_join, &_condattr);

	}
	
	static mydeterm& getInstance(void) {
    	static mydeterm * determObject = NULL;
    	if(!determObject) {
      		void *buf = mmap(NULL, sizeof(mydeterm), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      		determObject = new(buf) mydeterm();
    	}
    	return * determObject;
  	}
	void finalize(void) {
	    WRAP(pthread_mutex_destroy)(&_mutex);
    	WRAP(pthread_cond_destroy)(&cond);
    	assert(_currthreads == 0);
 	 }

	// Add this thread to the list.
  	// When one thread is registered, no one else is running,
  	// there is no need to hold the lock.
  	void registerThread(int threadindex, int pid, int parentindex) {
		//printf("registerThread entering\n");
		ThreadEntry *entry;

		// Allocate memory to hold corresponding information.
    	void * ptr = allocThreadEntry(threadindex);
		
		entry = new (ptr) ThreadEntry(pid, threadindex);
		entry->tid_parent = parentindex;
		
		entry->status = STATUS_READY;
		
		// Add one entry according to their threadindex.
		// should have lock?
		lock();
		listInsertTail((list_t*)entry, _activeList);
		unlock();
//		printf("registerThread entering, %d\n", threadindex);
	}

	void deleteRegisterThread(int threadindex){
		ThreadEntry * entry = &_entries[threadindex];
    	ThreadEntry * parent = &_entries[entry->tid_parent];
    	ThreadEntry * nextentry;			
		

		lock();
		//tell parent thread, I have finished.
		//if(parent->status == STATUS_JOINING)
	
		printf("delete %d, %d, %d\n", parent->joinee_thread_index, threadindex, parent->status);	
		if ( parent->joinee_thread_index == threadindex)
		{
		
			WRAP(pthread_cond_broadcast)(&_cond_join);
		}
			
		// Decrease number of alive threads and fence.
	    if(_alivethreads > 0) {
     	// Since this thread is running with the token, no need to modify 
      	// _currthreads.
        	_alivethreads--;
           	//decrFence();
        }
		//activeList is NULL.
    	//nextentry = (ThreadEntry *)entry->next;
		//assert(nextentry != entry);
		assert(_activeList == NULL);
			
		//recycle memory, and delete threadentry from active list.	
		if(entry->next == entry->prev){
			_activeList = NULL;
		}else
			listRemoveNode((list_t*)entry);
		
		freeThreadEntry(entry);
		
		unlock();	
	}
	
	//notify parent thread, child thread has registered.
	inline void waitParentNotify(void){
		lock();
		_childregistered = true;
		WRAP(pthread_cond_signal)(&_cond_parent);
		unlock();
	}

	//parent thread is waiting until children thread registers.
	void waitChildRegistered(void){
		lock();
    	if (!_childregistered) {
      		WRAP(pthread_cond_wait)(&_cond_parent, &_mutex);
		}
    	_childregistered = false;
    	unlock();		
	}
	inline bool join(int guestindex, int myindex, bool wakeup){
		ThreadEntry *joinee;
		ThreadEntry *myentry;
		lock();
		joinee = (ThreadEntry *)&_entries[guestindex];
		myentry = (ThreadEntry *)&_entries[myindex];	
		
		if(joinee->status != STATUS_EXIT){
			myentry->status = STATUS_JOINING;
			myentry->joinee_thread_index = guestindex;
			
			printf("parent joining child, %d, %d, %d\n", guestindex, myindex, myentry->status);	
		}
		if(joinee->status != STATUS_EXIT){
		
			WRAP(pthread_cond_wait)(&_cond_join, &_mutex);	
		}
		unlock();
		return true;
	}		

private:

	//we can optimaze this. Create a pool to manage ThreadEntry.
	inline void * allocThreadEntry(int threadindex) {
		//printf("%d, %d\n", threadindex, _maxthreadentries);
		//assert(threadindex < _maxthreadentries);
		return (&_entries[threadindex]);
	}

	inline void freeThreadEntry(void *ptr) {
		ThreadEntry * entry = (ThreadEntry *) ptr;
		entry->status = STATUS_EXIT;
		// Do nothing now.
		return;
	}
	void *getSyncEntry(void * entry){
		void **ptr = (void **)entry;
		return (*ptr);
	}
	void setSyncEntry(void * originalEntry, void * newEntry){
		void **dest = (void**)originalEntry;
		*dest = newEntry;
		xmemory::mem_write(dest, newEntry);
	}
	
	void clearSyncEntry(void *originalEntry){
		void **dest = (void**)originalEntry;
	}
	inline void * freeSyncEntry(void * ptr){
		if(ptr != NULL){
			xmemory::getInstance().free(ptr);
			//InternalHeap::getInstance().free(ptr);		
		}
	}
	inline void * allocSyncEntry(int size){
		return xmemory::getInstance().malloc(size);
		//return InternalHeap::getInstance().malloc(size);
	}
	
  	inline void lock(void) {
    	WRAP(pthread_mutex_lock)(&_mutex);
  	}

  	inline void unlock(void) {
    	WRAP(pthread_mutex_unlock)(&_mutex);
  	}
};


#endif
