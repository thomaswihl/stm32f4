#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdint.h>
#if __ARM__
//inline int32_t atomic_add(volatile int32_t* v, int inc)
//{
//   int32_t t;
//   int tmp;

//   __asm__ __volatile__(
//                         "1:  ldrex   %0, [%2]        \n"
//                         "    add     %0, %0, %3      \n"
//                         "    strex   %1, %0, [%2]    \n"
//                         "    cmp     %1, #0          \n"
//                         "    bne     1b              \n"

//                         : "=&r" (t), "=&r" (tmp)
//                         : "r" (v), "r" (inc)
//                         : "cc", "memory");

//   return t;
//}

static inline void atomic_add(volatile int32_t* v, int inc)
{
        unsigned long tmp;
        int result;

        __asm__ __volatile__("@ atomic_add\n"
"1:     ldrex   %0, [%3]\n"
"       add     %0, %0, %4\n"
"       strex   %1, %0, [%3]\n"
"       teq     %1, #0\n"
"       bne     1b"
        : "=&r" (result), "=&r" (tmp), "+Qo" (*v)
        : "r" (v), "Ir" (inc)
        : "cc");
}
#endif  // ARM

#if __x86_64__ || __X86_32__ || __X86__
inline void atomic_add(volatile int32_t* v, int inc)
{
    *v += inc;
}
#endif  // X86

#endif // ATOMIC_H
