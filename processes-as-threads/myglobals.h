#ifndef MYGLOBAL_H__
#define MYGLOBAL_H__

#include <sys/mman.h>
#include "map_parse/pmparser.h"
#include "xdefines.h"
#include "mm.h"

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
		void *tmpZone= MM::mmapAllocateShared(USER_HEAP_SIZE*2);
		
		int j = 0;
		size_t offset = 0;
		for (j=0; j<maplist.length ; j++){
				
			maplist.pmparser_print(j, 0);
			if(strcmp(maplist.map[j].pathname, "[anonym*]")==0){
				memcpy((void*)((unsigned long)tmpZone+(unsigned long)offset), maplist.map[j].addr_start, maplist.map[j].length);
				
				if (munmap(maplist.map[j].addr_start, maplist.map[j].length) == -1){
					printf("munmap failed\n");
				}
				void *persistZone = mmap(maplist.map[j].addr_start, maplist.map[j].length, PROT_READ| PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
				if(persistZone == MAP_FAILED) {
 		       		printf("mmap error\n");
					abort();
 		     	}
				maplist.map[j].addr_map = persistZone;
				memcpy(maplist.map[j].addr_start, (void *)((unsigned long)tmpZone+(unsigned long)offset), maplist.map[j].length);
				offset += maplist.map[j].length;
			}else{
				int fd2;

   				fd2 = open(maplist.map[j].pathname, O_RDWR);
				if (munmap(maplist.map[j].addr_start, maplist.map[j].length) == -1){
					printf("munmap failed\n");
				}
				mmap(maplist.map[j].addr_start, maplist.map[j].length, PROT_READ| PROT_WRITE, MAP_SHARED|MAP_FIXED, fd2, 0);
				close(fd2);
			}
		}	
		MM::mmapDeallocate(tmpZone, USER_HEAP_SIZE*2);
	}

	void begin(bool cleanup){
	}

	void checkandcommit(bool update){
	}
};

#endif
