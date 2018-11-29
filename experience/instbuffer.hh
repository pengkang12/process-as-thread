#if !defined(INSTBUFFER_H)
#define INSTBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>

typedef struct instpos {
  uint8_t* iaddr;
  uint64_t rel_address;
  unsigned int length;
} InstPos;

class instbuffer {

  public:

    /*
       static instbuffer& getInstance() {
       static char buf[sizeof(instbuffer)];
       static instbuffer* theOneTrueObject = new (buf) instbuffer();
       return *theOneTrueObject;
       }
       */
    instbuffer() {
      isinit = false;
    }
    
    bool isInit() { return isinit; }
    void init(size_t buffersize, uint8_t* inst) {
      total = buffersize;
      size = buffersize * sizeof(InstPos);
      // aligned up to pagesize
      int protInfo = PROT_READ | PROT_WRITE;
      int sharedInfo = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
      start = (InstPos*)mmap(NULL, size, protInfo, sharedInfo, -1, 0); 
      idx = 0;
      cur = 0;
      faultyinst = inst;
      isinit = true;
    }

    void addNewInst(uint8_t* iaddr, uint64_t rel_address, unsigned int length){
      InstPos* newinst = &start[cur++];
      newinst->iaddr = iaddr;
      newinst->rel_address = rel_address;
      newinst->length = length;
      if(rel_address == (intptr_t)faultyinst) {
        faultyidx = cur - 1;
        idx = faultyidx;
      }
    }

    InstPos* findInstByAddr(uint8_t* iaddr){
      // skip some checks in the middle, since the max length of instruction is 15
      unsigned int current = iaddr - start[0].iaddr;
      if(current >= cur) current = cur - 1;
      if(start[current].iaddr == iaddr){
        return &start[current]; 
      }

      // do not combine together to reduce some checks 
      if(start[current].iaddr > iaddr){
        while(true) {
          current--;
          if(start[current].iaddr == iaddr){
            return &start[current]; 
          } else if (start[current].iaddr < iaddr) {
            break;
          }
        }
      } else {
        while(true) {
          current++;
          if(start[current].iaddr == iaddr){
            return &start[current]; 
          } else if (start[current].iaddr > iaddr) {
            break;
          }
        }
      }

      return NULL;
    }

    InstPos* findInstByRelAddr(uint64_t rel_address){
      // skip some checks in the middle, since the max length of instruction is 15
      unsigned int current = rel_address - start[0].rel_address;
      if(current >= cur) current = cur - 1;
      //fprintf(stderr, "current is %d cur is %lu, start[current].rel_address is %lx\n", current, cur, start[current].rel_address);
      if(start[current].rel_address == rel_address){
        return &start[current]; 
      }

      // do not combine together to reduce some checks 
      if(start[current].rel_address > rel_address){
        while(true) {
          current--;
          if(start[current].rel_address == rel_address){
            return &start[current]; 
          } else if (start[current].rel_address < rel_address) {
            break;
          }
        }
      } else {
        while(true) {
          current++;
          if(start[current].rel_address == rel_address){
            return &start[current]; 
          } else if (start[current].rel_address > rel_address) {
            break;
          }
        }
      }

      return NULL;
    }

    InstPos* getNext(){
      if(idx < total - 1){
        return &start[++idx];
      } 

      return NULL;
    }

    InstPos* getPrev(){
      if((long)idx > 0){
        return &start[--idx];
      } 

      return NULL;
    }

    InstPos* getCurrent(){
      return &start[idx];
    }

    void resetToFaultyInst(){
      idx = faultyidx;
    }

    InstPos* setFaultyInstByRelAddr(uint64_t rel_address){
      InstPos* inst = findInstByRelAddr(rel_address);
      idx = faultyidx = inst - start;
      faultyinst = inst;
      return inst;
    }

    InstPos* setFaultyInstByAddr(uint8_t* address){
      InstPos* inst = findInstByAddr(address);
      idx = faultyidx = inst - start;
      faultyinst = inst;
      return inst;
    }

  private:

    bool isinit;
    InstPos* start;
    size_t idx;
    size_t cur;
    size_t total;
    size_t size;
    size_t faultyidx;
    void* faultyinst;

};

#endif
