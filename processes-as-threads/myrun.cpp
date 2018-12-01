#include "myrun.h"


size_t myrun::_thread_index;
size_t myrun::_master_thread_id;
//bool myrun::_fence_enabled;

volatile bool myrun::_initialized = false;
//volatile bool myrun::_protection_enabled;
size_t myrun::_children_threads_count = 0;
size_t myrun::_lock_count = 0;
//bool myrun::_token_holding;
pthread_condattr_t myrun::_condattr;
pthread_mutexattr_t myrun::_mutexattr;
