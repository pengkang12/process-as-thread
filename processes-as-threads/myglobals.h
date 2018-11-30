#ifndef MYGLOBAL_H__
#define MYGLOBAL_H__

#include <sys/mman.h>
#include "map_parse/pmparser.h"
#include "xdefines.h"
#include "mm.h"
#include "libfuncs.h"

class xglobals {
private:
public:
	xglobals(){}
	void initialize(){
		class mapparser maplist;
		maplist.pmparser_parse(-1);
		//we need to store global map file to stack. Because we need to rewrite global value;
		char _backingFname[L_tmpnam];
 	    sprintf(_backingFname, "dthreadsMXXXXXX");
		int fd = mkstemp(_backingFname);
		if (fd == -1) {
	       fprintf(stderr, "failed to make persistent file.\n");
	       ::abort();
	    }
		int (*open2)(const char *, int, ...) = open;
        void* (*(mmap2))(void*, size_t, int, int, int, off_t) = mmap;
		void* (*memcpy2)(void*, const void*, long unsigned int) = memcpy;	
		
		int j = 0;
		for (j=0; j<maplist.length ; j++){
				
			void *tmpZone= MM::mmapAllocatePrivate(USER_HEAP_SIZE*2);
			//maplist.pmparser_print(j, 0);
			int fd1 = -1;
			{
//				if (j==2) continue;	
				if (maplist.map[j].length < USER_HEAP_SIZE)
				{
					memcpy2((void*)tmpZone, maplist.map[j].addr_start, maplist.map[j].length);
				}
				if (munmap(maplist.map[j].addr_start, maplist.map[j].length) == -1){
					printf("munmap failed\n");
				}
			
				void *persistZone = NULL;
				if (strlen(maplist.map[j].pathname)>0) // (strcmp(__FILE__, maplist.map[j].pathname) != 0))
				{

					fd1 = open2(maplist.map[j].pathname,  O_RDONLY);	
					if(fd1 == -1) {
						perror("open failed");
						fprintf(stderr, "errno = %d, filepath = %s\n", errno, maplist.map[j].pathname);
					}
					persistZone = mmap2(maplist.map[j].addr_start, maplist.map[j].length,PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_FIXED, fd1, maplist.map[j].offset);
				
				}else{
					//persistZone = MM::mmapAllocateShared(maplist.map[j].length, maplist.map[j].addr_start, fd, maplist.map[j].offset);
					persistZone = mmap2(maplist.map[j].addr_start, maplist.map[j].length, PROT_READ| PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_FIXED, fd, maplist.map[j].offset);
				}	
				if(persistZone == MAP_FAILED) {
					perror("mmap failed");
					abort();
 		     	}
				
				maplist.map[j].addr_map = persistZone;
				if (maplist.map[j].length < USER_HEAP_SIZE)
				{
//					if(j !=0 )
						memcpy2(maplist.map[j].addr_start, tmpZone, maplist.map[j].length);
				}
			}
			MM::mmapDeallocate(tmpZone, USER_HEAP_SIZE*2);
			if(fd1 != -1)
					close(fd1);
		}
	}

	void begin(bool cleanup){
	}

	void checkandcommit(bool update){
	}
};

#endif
