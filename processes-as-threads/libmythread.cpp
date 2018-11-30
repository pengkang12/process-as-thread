#include <pthread.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include "xmemory.h"
#include "myrun.h"

extern "C" {
  bool initialized = false;
  unsigned long textStart, textEnd;
  unsigned long globalStart, globalEnd;
  unsigned long heapStart, heapEnd;
	enum { InitialMallocSize = 1024 * 1024 * 1024 };
}

runtime_data_t * global_data;

__attribute__((constructor)) void initialize() {

	init_real_functions();
	
    xmemory::getInstance().initialize();
	
	global_data = (runtime_data_t *)mmap(NULL, xdefines::PageSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	printf("global_data address is %p\n", (void *)global_data);
	//parent thread index should be 1.
	global_data->thread_index = 1;
	myrun::initialize();
	initialized = true;
	
	void *ptr = (void *)malloc(100);	
	printf("this is my first time print %p\n", (void*)ptr);
}

__attribute__((destructor))void finalize(){
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

int pthread_create (pthread_t * tid, const pthread_attr_t * attr, void *(*fn) (void *), void * arg) {
	//assert(initialized);
	if(initialized) {
		*tid = (pthread_t)myrun::spawn(fn, arg);	
	}
	//printf("haha, this is my first step\n");
	return 0;
}

int pthread_join(pthread_t tid, void ** val) {
	//assert(initialized);
	if(initialized) {
		myrun::join((void*)tid, val);
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
		myrun::cond_init((void*) cond);
	}
	return 0;
}

int pthread_cond_broadcast(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
		myrun::cond_broadcast((void*) cond);	
	}
	return 0;
}

int pthread_cond_signal(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
		myrun::cond_signal((void *)cond);
	}
	return 0;
}

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex) {
	//assert(initialized);
	if(initialized) {
		myrun::cond_wait((void *)cond, (void*)mutex);
	}
	return 0;
}

int pthread_cond_destroy(pthread_cond_t * cond) {
	//assert(initialized);
	if(initialized) {
		myrun::cond_destroy(cond);
	}
	return 0;
}

// Add support for barrier functions
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t * attr, unsigned int count) {
	//assert(initialized);
	if(initialized) {
		myrun::barrier_init(barrier, count);
	}
	return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
	//assert(initialized);
	if(initialized) {
		myrun::barrier_destroy(barrier);
	}
	return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
	//assert(initialized);
	if(initialized) {
		myrun::barrier_wait(barrier);
	}
	return 0;
}
