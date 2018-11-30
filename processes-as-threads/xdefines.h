// -*- C++ -*-

/*
 
  Copyright (c) 2007-2012 Emery Berger, University of Massachusetts Amherst.

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

#ifndef _XDEFINES_H_
#define _XDEFINES_H_

#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ucontext.h>
#include <pthread.h>
#include <new>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "libfuncs.h"

/*
 * @file   xdefines.h   
 * @brief  Global definitions for Sheriff-Detect and Sheriff-Protect.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 */ 

#define gettid() syscall(SYS_gettid)
extern "C"
{
 
  typedef void * threadFunction (void *);
  
  extern unsigned long textStart, textEnd;
  extern unsigned long globalStart, globalEnd;
  extern unsigned long heapStart, heapEnd;
  //extern bool initialized;

  enum { USER_HEAP_BASE     = 0x40000000 }; // 1G
  enum { USER_HEAP_SIZE = 1048576UL * 8192  * 8}; // 8G
  //enum { USER_HEAP_SIZE = 1048576UL * 8192  * 2}; // 8G
  
  enum { MAX_USER_SPACE     = USER_HEAP_BASE + USER_HEAP_SIZE };
  enum { INTERNAL_HEAP_BASE = 0x100000000000 };
  enum { CACHE_STATUS_BASE   = 0x200000000000 };
  enum { CACHE_TRACKING_BASE = 0x200080000000 };
  enum { MEMALIGN_MAGIC_WORD = 0xCFBECFBECFBECFBE };
  enum { NUM_HEAPS = 128 };
  
  
  inline size_t alignup(size_t size, size_t alignto) {
    return ((size + (alignto - 1)) & ~(alignto -1));
  }
  
  inline size_t aligndown(size_t addr, size_t alignto) {
    return (addr & ~(alignto -1));
  }

	inline size_t getHeapId() {
		return gettid()%NUM_HEAPS; 
	}
  
};

class xdefines {
public:
	// FIXME: change it in order to evaluate the performance in a stable way 
	enum { HARDWARE_CORES_NUM = 32 };
//	enum { HARDWARE_CORES_NUM = 48 };
  enum { STACK_SIZE = 1024 * 1024 };
  enum { PHEAP_CHUNK = 1048576 };

  enum { INTERNALHEAP_SIZE = 1048576UL * 1024 * 8};
  enum { PAGE_SIZE = 4096UL };
  enum { PageSize = 4096UL };
  enum { PAGE_SIZE_MASK = (PAGE_SIZE-1) };
  
  enum { MAX_THREADS = 4096 };
	
	// We only support 64 heaps in total.
  enum { MAX_ALIVE_THREADS = NUM_HEAPS };
  enum {MAX_GLOBALS_SIZE = 1048576UL * 40};  
};

typedef struct runtime_data{
	volatile unsigned long thread_index;
}runtime_data_t;

extern runtime_data_t *global_data;
#endif
