/* 
  Copyright (c) 2012 , University of Massachusetts Amherst.

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
#ifndef _MM_H_
#define _MM_H_
#include <sys/types.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

class MM {
public:
  #define ALIGN_TO_CACHELINE(size) (size%64 == 0 ? size:(size+64)/64*64)

  static void mmapDeallocate (void * ptr, size_t sz) {
    munmap(ptr, sz);
  }

  static void * mmapAllocateShared (size_t sz,
         void * startaddr = NULL, 
         int fd = -1)
  {
    return allocate (true, sz, fd, startaddr);
  }

  static void * mmapAllocatePrivate (size_t sz,
         void * startaddr = NULL, 
         int fd = -1)
  {
    return allocate (false, sz, fd, startaddr);
  }

private:

  static void * allocate (bool isShared,
        size_t sz,
        int fd,
        void * startaddr) 
  {
    int protInfo   = PROT_READ | PROT_WRITE;
    int sharedInfo = isShared ? MAP_SHARED : MAP_PRIVATE;
    sharedInfo     |= ((fd == -1) ? MAP_ANONYMOUS : 0);
    sharedInfo     |= ((startaddr != NULL) ? MAP_FIXED : 0);

    void * ptr =  mmap(startaddr, sz, protInfo, sharedInfo, fd, 0);
    if(ptr == MAP_FAILED) {
      fprintf(stderr, "Couldn't do mmap %s : startaddr %p, sz %lx\n", strerror(errno), startaddr, sz);
      abort();
    }

    return ptr;
  }

};

#endif /* _MM_H_ */

