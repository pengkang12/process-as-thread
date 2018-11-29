#if !defined(DOUBLETAKE_XDEFINES_H)
#define DOUBLETAKE_XDEFINES_H

#include <stddef.h>
#include <ucontext.h>
#include <stdint.h>
#include <string.h>

#include "list.hh"
#include "instbuffer.hh"

/*
 * @file   xdefines.h
 * @brief  Global definitions for Sheriff-Detect and Sheriff-Protect.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 */

#if 0
//#define DEBUG(...) 
typedef struct runtime_data {
  volatile unsigned long thread_index;
  volatile unsigned long threads;
  struct runtime_stats stats;
} runtime_data_t;

extern runtime_data_t *global_data;
#endif
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

/*
#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
#define hash_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define hash_map __gnu_cxx::hash_map
#endif
*/
extern "C" {
extern size_t __max_stack_size;
typedef void* threadFunction(void*);
extern char* getThreadBuffer();
extern void jumpToFunction(ucontext_t* cxt, unsigned long funcaddr);
#ifdef DETECT_USAGE_AFTER_FREE
extern bool addThreadQuarantineList(void* ptr, size_t size);
#endif

typedef enum e_thrsynccmd {
    E_SYNC_SPAWN = 0, // Thread creation
    //  E_SYNC_COND_SIGNAL,// conditional signal
    //  E_SYNC_COND_BROADCAST,// conditional broadcast
    //  E_SYNC_COND_WAIT,// conditional wait
    //  E_SYNC_COND_WAKEUP,// conditional wakeup from waiting
    E_SYNC_BARRIER,   // barrier waiting

    E_SYNC_RWLOCK_LOCK = 2,
    E_SYNC_RWLOCK_UNLOCK,
    E_SYNC_RWLOCK_RDLOCK,
    E_SYNC_RWLOCK_TIMEDRDLOCK,
    E_SYNC_RWLOCK_TRYRDLOCK,
    E_SYNC_RWLOCK_WRLOCK,
    E_SYNC_RWLOCK_TIMEDWRLOCK,
    E_SYNC_RWLOCK_TRYWRLOCK,

    E_SYNC_MUTEX_LOCK = 10,
    E_SYNC_MUTEX_TRY_LOCK,
    E_SYNC_COND_WAIT, 
    E_SYNC_MUTEX_UNLOCK,
    E_SYNC_COND, // We actually don't need to maintain a list of conditional variable

    E_SYNC_THREAD, // Not a actual synchronization event, but for each thread.
    //  E_SYNC_KILL, // Inside the critical section.
} thrSyncCmd;

#ifdef SINGLELL
typedef struct SyncEvent {
  struct syncEvent *next;
  void *thread;
  void *eventlist;
  void *syncvar;
  thrSyncCmd synccmd;
  int ret;
} SyncEvent;
#else
typedef struct SyncEvent {
  list_t list;
  // Which thread is performing synchronization?
  void* thread;
  void* eventlist;
  thrSyncCmd synccmd;
  void*      syncvar;
  int ret; // used for mutex_lock
};
#endif

// A project can have multiple problems.
// So we make them to use different bits.
typedef enum e_faultyObjectType {
  OBJECT_TYPE_NO_ERROR = 0, // No error for this object
  OBJECT_TYPE_OVERFLOW = 1,
  OBJECT_TYPE_USEAFTERFREE = 2,
  OBJECT_TYPE_LEAK = 4,
  OBJECT_TYPE_WATCHONLY = 8,
  OBJECT_TYPE_DATATRACE = 16
} faultyObjectType;

inline size_t alignup(size_t size, size_t alignto) {
  return ((size + (alignto - 1)) & ~(alignto - 1));
}

inline size_t aligndown(size_t addr, size_t alignto) { return (addr & ~(alignto - 1)); }

struct watchpointsInfo {
  unsigned long count;
  // Current hardware only support 4 watchpoints in total.
  unsigned long wp[4];
};

#define WP_START_OFFSET sizeof(unsigned long)

union semun {
  int val;                /*  Value for SETVAL */
  struct semid_ds *buf;   /*  Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array; /*  Array for GETALL, SETALL */
  struct seminfo  *__buf; /*  Buffer for IPC_INFO (Linux-specific) */
}; 

struct freeObject {
  void* ptr;
  union {
    int owner; // which thread is using this heap.
    size_t size;
  };
};

//************** For binary analysis ******************/
typedef struct stack_frame {
  // values in registers
  gregset_t regs;
} sFrame;

enum ConfirmedResult {
  CONFIRMED_INDEX_1 = 0,
  CONFIRMED_INDEX_64 = 63,
  CONFIRMED_INDEX_INVALID = 64,
  CONFIRMED_INDEX_NONE
};

typedef struct stack_info {
  // memory access value
  long value;
  // synchronization number
  int sync_num;
  SyncEvent* sync_event[5];
  // call stack
  int level;
  sFrame callstack[32]; 
} CallStackInfo;

// instructions need to be comfirmed
typedef struct _suspectinst {
  // which indicates the access order of instruction
  int accesscounter;
  // how many instructions
  int instidx;
  // how many time we should replay to confirm instructions
  int replaytime;
  // last access instruction, 
  // this is one time data, destory it after use
  int confirmedinstidx;
  // real instruction
  void* inststack[128];
  // access order, which one to one mapping with inststack
  int instcounter[128];
  // perf fd to be used for enable and disable
  int instfd[128];
  // the context of instruction
  sFrame context[128];
} suspectInst;

typedef enum _sitetype {
  SITE_TYPE_ROOT = 0,
  SITE_TYPE_CHILD,
  SITE_TYPE_ROOT_SUBLEVEL,
  SITE_TYPE_CHANGE_REG
} sitetype;

// site in root cause trace 
typedef struct _siteinfo {
  // parent node
  struct _siteinfo * parent;
  // memory address
  void* faultyAddr;
  //faulty instruction
  InstPos* faultyInst;
  // current instruction address
  void* instruction_address;
  // how many times the instruction is triggered 
  // this is stop position for tracing multiple level root causes
  pid_t tid;
  int breakpoint_fd;
  unsigned int triggered_times;
  unsigned int total_triggered_times;
  // current site type
  sitetype type;
  // how many children?
  short refcounter;
  // whether it is checked
  short isChecked;
  // register which is tracked 
  uint32_t reg; 
  // if it is confirmed in replay, 
  // we would save call stack
  int level;
  int level_position;
  sFrame callstack[32]; 

  void addSiteInfo(struct _siteinfo * new_parent, void* new_faultyAddr, 
      InstPos* new_faultyInst, void* inst_address, bool new_isChecked, uint32_t new_reg, sitetype new_type,
      int new_level, int new_level_position, sFrame* new_callstack, pid_t new_tid=0) {
    parent = new_parent;
    type = new_type;
    faultyAddr = new_faultyAddr;
    faultyInst = new_faultyInst;
    instruction_address = inst_address;
    tid = 0;
    breakpoint_fd = -1;
    triggered_times = 0;
    total_triggered_times = 0;
    isChecked = new_isChecked;
    reg = new_reg;
    level = new_level;
    level_position = new_level_position;
    if(new_callstack != NULL && level != 0){
      memcpy(callstack, new_callstack, level * sizeof(sFrame));
    }
    
    if(parent) { 
      parent->refcounter++;
      tid = parent->tid;
    } 

    if(new_tid) {
      tid = new_tid;
    }
  }
    
} siteInfo;

// root cause information
typedef struct _rctrace {
  // how many root cause
  int rcIdx;
  // currently check info
  siteInfo* cur;
  // total root cause info
  siteInfo si[64];
} rcTrace;

typedef enum _trackcmd {
  TRACK_CMD_NONE = 0,
  TRACK_CMD_CHANGE_REG,
  TRACK_CMD_WALKING,
  TRACK_CMD_CONFIRM,
  TRACK_CMD_CONFIRM_WITH_CXT,
  TRACK_CMD_CONFIRM_WITH_CXT_BB,
  TRACK_CMD_ADDRESS,
  TRACK_CMD_WATCH_ADDRESS,
  TRACK_CMD_FAIL,
  TRACK_CMD_SUCCESS
} trackcmd;

typedef enum _faultytype {
  FAULTY_TYPE_NONE = 0,
  FAULTY_TYPE_SEGV,
  FAULTY_TYPE_ASSERT,
  FAULTY_TYPE_ABORT,
  FAULTY_TYPE_FPE
}faultyType;

typedef struct _trackctl {
  // track command
  trackcmd cmd; 
  // faulty type
  faultyType type; 

  /** they are intermediate value */
  siteInfo* root_level;
  // current working faulty instruction
  void* faultyip;
  // current working faulty address
  void* faultyaddr;
  // faulty call stack
  int checkedlevel;
  int cslevel;
  sFrame callstack[32]; 
  // synchronization number
  int sync_num;
  SyncEvent* sync_event[5];
  /** they are intermediate value end */

  // too cause trace
  rcTrace trace;
  // confirm instructions
  suspectInst sInst;
} trackctl;
//************** For binary analysis end ******************/

#define OUTFD 2
#define LOG_SIZE 4096
#define LOCKSETSIZE 5

typedef void (*custom_sa_handler)(int);
typedef void (*custom_sa_sigaction)(int, siginfo_t *, void *); 

}; // extern "C"

class xdefines {
public:
  enum { USER_HEAP_SIZE = 1048576UL * 8192 }; // 8G
  //  enum { USER_HEAP_SIZE     = 1048576UL * 1024 }; // 8G
  enum { USER_HEAP_BASE = 0x100000000 }; // 4G
  enum { MAX_USER_SPACE = USER_HEAP_BASE + USER_HEAP_SIZE };
  enum { INTERNAL_HEAP_BASE = 0x100000000000 };
  // enum { INTERNAL_HEAP_BASE = 0xC0000000 };
  enum { INTERNAL_HEAP_SIZE = 1048576UL * 4096 };
  enum { INTERNAL_HEAP_END = INTERNAL_HEAP_BASE + INTERNAL_HEAP_SIZE };

  enum { QUARANTINE_BUF_SIZE = 1024 };

	enum { STACK_SIZE = 0x21000 };
  // If total free objects is larger than this size, we begin to
  // re-use those objects
  enum { QUARANTINE_TOTAL_SIZE = 1048576 * 16 };

  // 128M so that almost all memory is allocated from the begining.
  enum { USER_HEAP_CHUNK = 1048576 * 4 };
  enum { INTERNAL_HEAP_CHUNK = 1048576 };
  enum { OBJECT_SIZE_BASE = 16 };
  // enum { MAX_OBJECTS_USER_HEAP_CHUNK = USER_HEAP_CHUNK/MINIMUM_OBJECT_SIZE };
  // enum { TOTAL_USER_HEAP_HUNKS = USER_HEAP_SIZE/USER_HEAP_CHUNK };

  enum { WATCHPOINT_TRAP_MAP_SIZE = 528384 };

  // 4 bytes is representing by 1 bit. If bit is 0, it is not a canary word.
  // Otherwise, it is a canary word.
  enum { BIT_SECTOR_SIZE = 32 };
  
  enum { MAX_CPU_NUMBER = 32 };
  enum { MAX_WATCHPOINTS = 3 };
  enum { PageSize = 4096UL };
  enum { PAGE_SIZE_MASK = (PageSize - 1) };

  // This is a experimental results. When we are using a larger number, rollback may fail.
  // Don't know why, although maximum number of semaphore is close to 128.
  enum { MAX_ALIVE_THREADS = 1025 };

  // Start to reap threads when reaplable threas is larer than that.
  //enum { MAX_REAPABLE_THREADS = 8 };
  enum { MAX_REAPABLE_THREADS = (MAX_ALIVE_THREADS - 10) };
  enum { NUM_HEAPS = MAX_ALIVE_THREADS };
  enum { SYNCMAP_SIZE = 4096 };
  enum { THREAD_MAP_SIZE = 1024 };
  enum { MAX_STACK_SIZE = 0xa00000UL };  // 64pages
  enum { TEMP_STACK_SIZE = 0xa00000UL }; // 64 pages
  enum { NUM_GLOBALS = 128 }; // At least, we need app globals, libc globals and libthread globals.
  // enum { MAX_GLOBALS_SIZE = 1048576UL * 10 };
  enum { CACHE_LINE_SIZE = 64 };

  /**
   * Definition of sentinel information.
   */
  enum { WORD_SIZE = sizeof(size_t) };
  enum { WORD_SIZE_MASK = WORD_SIZE - 1 };
  enum { SENTINEL_SIZE = WORD_SIZE };
  enum { MAGIC_BYTE_NOT_ALIGNED = 0x7E };
  enum { FREE_OBJECT_CANARY_WORDS = 16 };
  enum { FREE_OBJECT_CANARY_SIZE = 16 * WORD_SIZE };
  enum { CALLSITE_MAXIMUM_LENGTH = 10 };

  // FIXME: the following definitions are sensitive to
  // glibc version (possibly?)
  enum { MAX_OPENED_FILES = 1048576 };
  enum { FILES_MAP_SIZE = 4096 };
  enum { MEMTRACK_MAP_SIZE = 4096 };
  enum { INSTRUCTION_MAP_SIZE = 512 };
  enum { FOPEN_ALLOC_SIZE = 0x228 };
  enum { FOPEN_BUFFER = 0x200 };
  enum { DIRS_MAP_SIZE = 1024 };
  enum { DIROPEN_ALLOC_SIZE = 0x8030 };

  //socket config
  enum { SOCKET_MAP_SIZE = 2048 };
  enum { SOCKET_OPT_SIZE = 64 };
  enum { SOCKET_CACHE_SIZE = 1048576 * 3 };
  //enum { SOCKET_CACHE_SIZE = 524288 * 1 }; // 0.5M
  enum { MAX_SOCKET_NUMBER = 800 };
  enum { EPOLLWAIT_TIMEOUT = 500 };

  //  enum { MAX_RECORD_ENTRIES = 0x1000 };
  enum { BACKUP_SYSCALL_ENTRIES = 10 };
  enum { MAX_SYSCALL_ENTRIES = 0x1000000 };
  enum { MAX_RECORD_SYSCALL_ENTRIES = 0x100000 };
  enum { MAX_DEFER_SYSCALL_ENTRIES = 0x100000 };
#ifdef DUMP_LOG_TODISK
  enum { MAX_SYNCEVENT_ENTRIES = 0x300000 };
#else
  enum { MAX_SYNCEVENT_ENTRIES = 0x1000000 };
#endif
  enum { MAX_SYNCVARIABLE_ENTRIES = 0x10000 };
  enum { MAX_FCNTL_RECORD_LOCK_ENTRIES = 0x100000 };

#ifdef X86_32BIT
  enum { SENTINEL_WORD = 0xCAFEBABE };
  enum { MEMALIGN_SENTINEL_WORD = 0xDADEBABE };
#else
  enum { SENTINEL_WORD = 0xCAFEBABECAFEBABE };
  enum { MEMALIGN_SENTINEL_WORD = 0xDADEBABEDADEBABE };
#endif
};

#endif