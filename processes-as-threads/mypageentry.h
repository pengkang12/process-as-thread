#ifndef MYPAGEENTRY_H__
#define MYPAGEENTRY_H__

#include <errno.h>
#include <stdlib.h>
#include <new>


typedef struct mypageinfo {
	int pageNo;
	
	int version;
	
	void *pageStart;
	bool isUpdated;
	bool isShared;
	bool release;
}mypageinfo_t;

class mypageentry{
private:
	int _total;

	//Current index of entry that need to be allocated.
	int _current;

	mypageinfo_t * _start;
	
	enum{
		PAGE_ENTRY_NUM = 800000	
	};
public:
	mypageentry(){
		_start = NULL;
		_current = 0;
		_total = 0;	
	}
	
	static mypageentry &getInstance(void){
		static char buf[sizeof(mypageentry)];
		static mypageentry *theOneTrueObject = new (buf) mypageentry();
		return *theOneTrueObject;
	}
	
	void initialize(void){
		void *start = NULL;
		
		start = mmap(NULL, PAGE_ENTRY_NUM * sizeof(mypageinfo_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (start == NULL){
			fprintf(stderr, "%d faild to allocate page entries: %s\n", getpid(), strerror(errno));
			::abort();
		}
		
		_current = 0;
		_total = PAGE_ENTRY_NUM;
		_start = (mypageinfo_t *)start; 
		printf("-------------page entry initialized\n");
	}
	
	mypageinfo_t * alloc(void){
		mypageinfo_t *entry = NULL;
		if(_current < _total){
			entry = &_start[_current];
			_current ++;
		}else{
			fprintf(stderr, "no enough page entry, now _current is %d, _total is %d\n", _current, _total);
			::abort();
		}
		return entry;
	}
	
	void cleanup(void){
		_current = 0;	
	}
};

#endif /* __MYPAGEENTRY)H__*/
