// -*- C++ -*-

/*
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

/**
 * @class atomic
 * @brief A wrapper class for basic atomic hardware operations.
 *
 * @author Tongping Liu <http://www.cs.utsa.edu/~tongpingliu>
 */

#ifndef __ATOMIC_H
#define __ATOMIC_H

  inline bool atomic_compare_and_swap(volatile unsigned long * ptr, unsigned long oldval, unsigned long newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
  }

  inline unsigned long atomic_exchange(volatile unsigned long * oldval,
      unsigned long newval) {
#if defined(X86_32BIT)
    asm volatile ("lock; xchgl %0, %1"
#else
    asm volatile ("lock; xchgq %0, %1"
#endif
        : "=r" (newval)
        : "m" (*oldval), "0" (newval)
        : "memory");
    return newval;
  }

  // Atomic increment 1 and return the original value.
  inline unsigned long atomic_increment_and_return(volatile unsigned long * obj) {
    unsigned long i = 1;
#if defined(X86_32BIT)
    asm volatile("lock; xaddl %0, %1"
#else
    asm volatile("lock; xadd %0, %1"
#endif
        : "+r" (i), "+m" (*obj)
        : : "memory");
    return i;
  }

  inline void atomic_increment(volatile unsigned long * obj) {
    asm volatile("lock; incl %0"
        : "+m" (*obj)
        : : "memory");
  }

  inline void atomic_add(int i, volatile unsigned long * obj) {
#if defined(X86_32BIT)
    asm volatile("lock; addl %0, %1"
#else
    asm volatile("lock; add %0, %1"
#endif
        : "+r" (i), "+m" (*obj)
        : : "memory");
  }

  inline void atomic_decrement(volatile unsigned long * obj) {
    asm volatile("lock; decl %0;"
        : :"m" (*obj)
        : "memory");
  }

  // Atomic decrement 1 and return the original value.
  inline int atomic_decrement_and_return(volatile unsigned long * obj) {
    int i = -1;
#if defined(X86_32BIT)
    asm volatile("lock; xaddl %0, %1"
#else
    asm volatile("lock; xadd %0, %1"
#endif
        : "+r" (i), "+m" (*obj)
        : : "memory");
    return i;
  }

  inline unsigned long atomic_set(volatile unsigned long * oldval,
      unsigned long newval) {
    unsigned long value = newval;

#if defined(X86_32BIT)
    asm volatile ("lock; xchgl %0, %1"
#else
    asm volatile ("lock; xchgq %0, %1"
#endif
        : "=r" (value), "+m" (*oldval)    \
        : "0"(value) \
        : "memory");
    return value;
  }

  inline int atomic_read(const volatile unsigned long *obj) {
    return (*obj);
  }
  
  inline unsigned long atomic_test_and_set(volatile unsigned long *mem) {
    unsigned long ret;
    //fprintf(stderr, "Before lock: value is %d\n", *w);
#if defined(__i386__)
    asm volatile("lock; xchgl %0, %1":"=r"(ret), "=m"(*mem)
       :"0"(1), "m"(*mem)
       :"memory");
#else
    asm volatile("lock; cmpxchgl %2, %1"            \
           : "=a" (ret), "=m" (*mem)            \
           : "r" (1), "m" (*mem), "0" (0));
#endif
    return ret;
  }

  inline void atomic_memoryBarrier(void) {
    // Memory barrier: x86 only for now.
    __asm__ __volatile__ ("mfence": : :"memory");
  }

  inline void atomic_cpuRelax(void) {
    asm volatile("pause\n": : :"memory");
  }
#endif
