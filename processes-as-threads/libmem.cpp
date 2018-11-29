#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include "xmemory.h"

extern "C" {
  void initializer (void) __attribute__((constructor));
  void finalizer (void)   __attribute__((destructor));
  bool initialized = false;
  unsigned long textStart, textEnd;
  unsigned long globalStart, globalEnd;
  unsigned long heapStart, heapEnd;
	enum { InitialMallocSize = 1024 * 1024 * 1024 };

  void initializer (void) {
    // Using globals to provide allocation
    // before initialized.
    printf("we can start in there\n");
	init_real_functions();
 
    xmemory::getInstance().initialize();
    initialized = true;
	//fprintf(stderr, "Now we have initialized successfuuly\n"); 
  }

  void finalizer (void) {
    initialized = false;
  }

  // Temporary mallocation before initlization has been finished.
  static void * tempmalloc(int size) {
    static char _buf[InitialMallocSize];
    static int _allocated = 0;
  
    if(_allocated + size > InitialMallocSize) {
      printf("Not enough space for tempmalloc");
      abort();
    } else {
      void* p = (void*)&_buf[_allocated];
      _allocated += size;
      return p;
    }
  }

  /// Functions related to memory management.
  void * malloc (size_t sz) {
    void * ptr;
    if(sz == 0) {
      sz = 1;
    }

    // Align the object size. FIXME for now, just use 16 byte alignment and min size.
    if(!initialized) {
      if (sz < 16) {
        sz = 16;
      }
      sz = (sz + 15) & ~15;
      ptr = tempmalloc(sz);
    }
    else {
      ptr = xmemory::getInstance().malloc(sz);
    }

    if (ptr == NULL) {
      fprintf (stderr, "Out of memory!\n");
      ::abort();
    }
//    fprintf(stderr, "********MALLOC sz %ld ptr %p***********\n", sz, ptr);
    return ptr;
  }
  
  void * calloc (size_t nmemb, size_t sz) {
    void * ptr;
    
    ptr = malloc (nmemb * sz);
	  memset(ptr, 0, sz*nmemb);
    return ptr;
  }

  void free (void * ptr) {
    // We donot free any object if it is before 
    // initializaton has been finished to simplify
    // the logic of tempmalloc
    if(initialized) {
      xmemory::getInstance().free (ptr);
    }
  }

  size_t malloc_usable_size(void * ptr) {
    if(initialized) {
      return xmemory::getInstance().malloc_usable_size(ptr);
    }
    return 0;
  }

  // Implement the initialization
  void * memalign (size_t boundary, size_t size) {
    if(!initialized) {
			initializer();
    }
    return xmemory::getInstance().memalign(boundary, size);
   // return NULL;
  }

  void * realloc (void * ptr, size_t sz) {
    if(initialized) {
      return xmemory::getInstance().realloc(ptr, sz);
    }
    else {
      return tempmalloc(sz);
    }
  }
//int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *) {return 0;}
};

