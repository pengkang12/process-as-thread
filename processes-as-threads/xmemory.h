// -*- C++ -*-

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*
 * @file   xmemory.h
 * @brief  Memory management for all.
 *         This file only includes a simplified logic to detect false sharing problems.
 *         It is slower but more effective.
 * 
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 */ 

#ifndef _XMEMORY_H
#define _XMEMORY_H

//#include <signal.h>

#include <sys/types.h>
#include <unistd.h>
#include <set>
#include <sys/types.h>
#include <signal.h>

#include "myglobals.h"
#include "xpheap.h"
#include "xoneheap.h"
#include "xheap.h"
#include "objectheader.h"
#include "atomic.h"
#include "xdefines.h"
#include "ansiwrapper.h"
#include "mm.h"

class xmemory {
private:
	xglobals _globals;
  	// Private on purpose. See getInstance(), below.
  	xmemory(){}

  	#define CALLSITE_SIZE sizeof(CallSite)

  	objectHeader * getObjectHeader (void * ptr) {
    	objectHeader * o = (objectHeader *) ptr;
    	return (o - 1);
 	}

  	/// The protected heap used to satisfy big objects requirement. Less
  	/// than 256 bytes now.
  	xpheap<xoneheap<xheap> > _heap;		
public:
	
	// Just one accessor.  Why? We don't want more than one (singleton)
	// and we want access to it neatly encapsulated here, for use by the
	// signal handler.
	static xmemory& getInstance() {
		static char buf[sizeof(xmemory)];
		static xmemory * theOneTrueObject = new (buf) xmemory();
		return *theOneTrueObject;
	}

	void initialize() {
		// Initialize the heap and globals. Basically, we need 
		// spaces to hold access data for both heap and globals.
		_heap.initialize(USER_HEAP_SIZE);
		// I need to use once a malloc, otherwise I can't correct use heap. 
		void *a = (void*) malloc(32);
		_globals.initialize();  
}

  void finalize(void) {
  }
 
  inline void *malloc (size_t sz) {
    void * ptr = NULL;
		
		if(ptr == NULL) {
//  	  printf("xmemory::malloc sz %lx ptr %p\n", sz, ptr);
    	ptr = _heap.malloc(getHeapId(), sz);
  	}

    return (ptr);
  }

  inline void * realloc (void * ptr, size_t sz) {
    size_t s = getSize (ptr);

    void * newptr = malloc(sz);
    if(newptr && s != 0) {
      size_t copySz = (s < sz) ? s : sz;
      memcpy (newptr, ptr, copySz);
    }

    free(ptr);
    return newptr;
  }

  inline void * memalign(size_t boundary, size_t size) {
    void * ptr;
    unsigned long wordSize = sizeof(unsigned long);

    if(boundary < 8 || ((boundary & (wordSize-1)) != 0)) {
      fprintf(stderr, "memalign boundary should be larger than 8 and aligned to at least 8 bytes. Acutally %ld!!!\n", boundary);
      abort();
    }

    ptr = malloc(boundary + size);

    return ptr; 
  }

  inline void free (void * ptr) {
    if(ptr == NULL) 
      return;

    void * realPtr = ptr;
    _heap.free(getHeapId(), realPtr);
  }

  size_t malloc_usable_size(void * ptr) {
    return getSize(ptr);
  }
  
  	/// @return the allocated size of a dynamically-allocated object.
  	inline size_t getSize (void * ptr) {
    	// Just pass the pointer along to the heap.
    	return _heap.getSize (ptr);
  	}
	static inline void commit(bool update){
		//_heap.checkandcommit(update);
		//_globals.checkandcommit(update);
	}
	static inline void begin(bool cleanup){
		//_globals.begin(cleanup);
		//_heap.begin(cleanup);
	} 

	static void mem_write(void * dest, void *val){
			
	}
	
	// Page fault signal handler function to trap in SEGVs.	
	static void segactionHandle(int signum, siginfo_t * siginfo, void *context){
		void *addr = siginfo->si_addr; //address of access
		
		//Check if this signal is SEGV that we are supposed to trap.
		if (siginfo->si_code == SEGV_ACCERR){

			printf("---------this is SEGV signal-----------\n");
		}else if(siginfo->si_code == SEGV_MAPERR){
			fprintf(stderr, "%d: map error with addr %p!\n", getpid(), addr);
			::abort();	
		}else{
			fprintf(stderr, "%d: other access error with addr %p.\n", getpid(), addr);
			::abort();
		}
		
	}
		
	void installSignalHandler(){
		struct sigaction siga;
		sigemptyset(&siga.sa_mask);	
		
		//set the following signals to a set
		sigaddset(&siga.sa_mask, SIGSEGV);
		sigaddset(&siga.sa_mask, SIGALRM);
		//block siganl	
		sigprocmask(SIG_BLOCK, &siga.sa_mask, NULL);
		
		siga.sa_flags = SA_SIGINFO | SA_RESTART;
		siga.sa_sigaction = xmemory::segactionHandle;
		if (sigaction(SIGSEGV, &siga, NULL) == -1){
			fprintf(stderr, "sigaction(SIGSEGV)\n");
			exit(-1);	
		}
		//remove blocked signal
		sigprocmask(SIG_UNBLOCK, &siga.sa_mask, NULL);	
		printf("----------we successfully register signal for SEGV\n");
	}
};

#endif
