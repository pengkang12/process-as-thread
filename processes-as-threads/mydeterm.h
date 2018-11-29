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
    	inline ThreadEntry():prev(NULL), next(NULL) {}

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
	
	class LockEntry {
	public:
		volatile int total_users;
		volatile int last_thread;
		volatile bool is_acquired;
		volatile int lock_budget;
	};
	
	// When one thread is created, it will wait until all threads are created.
  	// The following two flag are used to indentify whether one thread can move on or not.
  	volatile bool _childregistered;
  	volatile bool _parentnotified;

	// All threads should be putted into this active list.
	// Here, header is not one node in the circular link list.
	// _activelist->next is the first node in the link list, while _activelist->prev is
	// the last node in the link list.
 	list_t *_activeList;
	ThreadEntry _entries[MAX_THREADS];

	size_t _coreNumber;	
	//how many do thread entries have in system. 
	size_t _maxthreadentries;
	
 	// Variables related to token pass and fence control
  	volatile ThreadEntry *_tokenpos;
  	
	//thread statistic information.
	volatile size_t _maxthreads;
  	volatile size_t _currthreads;
  	volatile bool _is_arrival_phase;
  	volatile size_t _alivethreads;
	
	mydeterm():
    //_condnum(0),
    //_barriernum(0),
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
	    //WRAP(pthread_mutex_destroy)(&_mutex);
    	//WRAP(pthread_cond_destroy)(&cond);
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
		printf("registerThread entering, %d\n", threadindex);
	}

	void deleteRegisterThread(int threadindex){
		ThreadEntry * entry = &_entries[threadindex];
    	ThreadEntry * parent = &_entries[entry->tid_parent];
    	ThreadEntry * nextentry;			
		
		DEBUG("%d: Deregistering", getpid());

		lock();
			
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
		
		//DEBUG("%d: deregistering. Token is passed to %d\n", getpid(), (ThreadEntry *)_tokenpos->threadindex);
		DEBUG("%d: delete registering thread\n", getpid());	
		unlock();	
		printf("Delete Registering thread, %d\n", threadindex);
	}

	//notify parent thread, child thread has registered.
	inline void notifyWaitingParent(void){
		lock();
		_childregistered = true;

		unlock();
	}

	//parent thread is waiting until children thread registers.
	void waitChildRegister(void){
		lock();
    	if (!_childregistered) {
      		//WRAP(pthread_cond_wait)(&_cond_parent, &_mutex);
      		if(!_childregistered) {
        		fprintf(stderr, "Child should be registered!!!!\n");
      		}
    	}
    	_childregistered = false;
    	unlock();		
	}
	
	LockEntry * lock_init(void* mutex){
		LockEntry * entry = allocLockEntry();
		entry->total_users = 0;
		entry->last_thread = 0;
		
		entry->is_acquired = false;
		
		//No one is the owner, we need to syncEntry;
		setSyncEntry(mutex, (void *)entry);
	
		return entry;
	}
	void lock_destroy(void * mutex){
		LockEntry * entry = (LockEntry*)getSyncEntry(mutex);
		setSyncEntry(mutex, NULL);
		freeSyncEntry(entry);	
	}
	inline bool lock_acquire(void *mutex){
		LockEntry *entry = (LockEntry *)getSyncEntry(mutex);
		bool result = true;
		if(entry == NULL){
			entry = lock_init(mutex);
		}
		
		if(entry->is_acquired == true)
			return false;
		entry->is_acquired = true;
		return result;
	}
	void lock_release(void *mutex){
		LockEntry *entry = (LockEntry *)getSyncEntry(mutex);
		entry->is_acquired = false;
	}

	
private:

	//we can optimaze this. Create a pool to manage ThreadEntry.
	inline void * allocThreadEntry(int threadindex) {
		assert(threadindex < _maxthreadentries);
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
	
		////
	
	}
	inline void * freeSyncEntry(void * ptr){
		if(ptr != NULL){
			InternalHeap::getInstance().free(ptr);		
		}
	}
	inline void * allocSyncEntry(int size){
		return InternalHeap::getInstance().malloc(size);
	}
	inline LockEntry *allocLockEntry(void){
		return ((LockEntry *) allocSyncEntry(sizeof(LockEntry)));
	}
	
  	inline void lock(void) {
    	//WRAP(pthread_mutex_lock)(&_mutex);
  	}

  	inline void unlock(void) {
    	//WRAP(pthread_mutex_unlock)(&_mutex);
  	}
};


#endif
