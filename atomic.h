#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdint.h>

inline int32_t atomic_add(volatile int32_t* v, int inc)
{
   int32_t t;
   int tmp;

   __asm__ __volatile__(
                         "1:  ldrex   %0, [%2]        \n"
                         "    add     %0, %0, %3      \n"
                         "    strex   %1, %0, [%2]    \n"
                         "    cmp     %1, #0          \n"
                         "    bne     1b              \n"

                         : "=&r" (t), "=&r" (tmp)
                         : "r" (v), "r" (inc)
                         : "cc", "memory");

   return t;
}

#endif // ATOMIC_H
