/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "System.h"

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern "C"
{

void __attribute__((naked)) Trap()
{
    register unsigned int* stackPointer asm("sp");
    System::instance()->handleTrap(stackPointer);
    __asm("ldr r0, =_exit");
    __asm("bx r0");
}

void __attribute__((interrupt)) SysTick()
{
    System::sysTick();
    //__asm("bx lr");
}

void __attribute__((interrupt)) Isr()
{
    System::instance()->handleInterrupt();
}

extern void (* const gIsrVectorTable[])(void);
__attribute__ ((section(".isr_vector_table")))
void (* const gIsrVectorTable[])(void) = {
    // 16 trap functions for ARM
    (void (* const)())&__stack_end, (void (* const)())&_start, Trap, Trap, Trap, Trap, Trap, 0,
    0, 0, 0, Trap, Trap, 0, Trap, SysTick,
    // 82 hardware interrupts specific to the STM32F407
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr
};

// required for C++
void* __dso_handle;

void __cxa_pure_virtual()
{
    std::printf("Pure fitual function called!\n");
    std::abort();
}

// init stuff
extern void __libc_init_array();
extern void __libc_fini_array();
extern int main();

// our entry point after reset
void _start()
{
    memcpy(&__data_start, &__data_rom_start, &__data_end - &__data_start);
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
    // calls __preinit_array, call _init() and then calls __init_array (constructors)
    __libc_init_array();

    // Make sure we have one instance of our System class
    assert(System::instance() != 0);

    int ret = main();

    // calls __fini_array and then calls _fini()
    __libc_fini_array();

    exit(ret);
}

void _init()
{
}

void _fini()
{
}

// os functions
#undef errno
extern int errno;

char *__env[1] = { 0 };
char **environ = __env;

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _close(int file)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    System::instance()->debugRead(ptr, len);
    return 0;
}

int _getpid(void)
{
    return 1;
}


int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _write(int file, const char *ptr, int len)
{
    return System::instance()->debugWrite(ptr, len);
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}


void* _sbrk(unsigned int incr)
{
    return System::increaseHeap(incr);
}

void _exit(int v)
{
    _write(1, "EXIT\n", 5);
    while (true)
    {
        __asm("wfi");
    }
}

}   // extern "C"

void *operator new(std::size_t size)
{
   return malloc(size);
}

void *operator new[](std::size_t size)
{
   return ::operator new(size);
}

void operator delete(void *mem)
{
   free(mem);
}

void operator delete[](void *mem)
{
   ::operator delete(mem);
}

namespace std
{
void __throw_bad_alloc()
{
    _write(1, "Out of memory, exiting.\n", 24);
    exit(1);
}

void __throw_length_error(const char*)
{
    _write(1, "Length error, exiting.\n", 24);
    exit(1);
}
}

System* System::mSystem;
char* System::mHeapEnd;
unsigned int System::mSysTick = 0;

char* System::increaseHeap(unsigned int incr)
{
    if (mHeapEnd == 0)
    {
        mHeapEnd = &__heap_start;
    }
    char* prevHeapEnd = mHeapEnd;
    if (mHeapEnd + incr >= &__heap_end)
    {
        _write(1, "ERROR: Heap full!\n", 18);
        abort();
    }
    memset(prevHeapEnd, 0, incr);
    mHeapEnd += incr;
    return prevHeapEnd;
}

uint32_t System::memFree()
{
    return &__heap_end - mHeapEnd;
}

uint32_t System::memUsed()
{
    return mHeapEnd - &__heap_start;
}

uint32_t System::stackFree()
{
    register char* stack __asm("r0");
    __asm volatile("mov r0, sp");
    return stack - &__stack_start;
}

uint32_t System::stackUsed()
{
    register char* stack __asm("sp");
    //__asm volatile("mov r0, sp");
    return &__stack_end - stack;
}

void System::postEvent(Event event)
{
    mEventQueue.push(event);
}

bool System::waitForEvent(Event &event)
{
    while (mEventQueue.used() == 0)
    {
        __asm("wfi");
    }
    return mEventQueue.pop(event);
}

System::System(BaseAddress base) :
    mBase(reinterpret_cast<volatile SCB*>(base)),
    mEventQueue(16)
{
    static_assert(sizeof(SCB) == 0x40, "Struct has wrong size, compiler problem.");
    // Make sure we are the first and only instance
    assert(mSystem == 0);
    mSystem = this;
    mBase->SHCSR.USGFAULTENA = 1;
    mBase->SHCSR.BUSFAULTENA = 1;
    mBase->SHCSR.MEMFAULTENA = 1;
    //mBase->CCR.UNALIGNTRP = 1;
    mBase->CCR.DIV0TRP = 1;
}

System::~System()
{
}

// The stack looks like this: (FPSCR, S15-S0) xPSR, PC, LR, R12, R3, R2, R1, R0
// With SP at R0 and (FPSCR, S15-S0) being optional
void System::handleTrap(TrapIndex index, unsigned int* stackPointer)
{
    static const char* TRAP_NAME[] =
    {
        nullptr,
        nullptr,
        "NMI",
        "Hard Fault",
        "Memory Management",
        "Bus Fault",
        "Usage Fault",
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "System Service Call",
        "Debug Monitor",
        nullptr,
        "Pending Request",
        nullptr
    };
    static_assert(sizeof(TRAP_NAME) / sizeof(TRAP_NAME[0]) == 16, "Not enough trap names defined, should be 16.");
    int intIndex = static_cast<int>(index);
    if (intIndex < 16 && TRAP_NAME[intIndex] != nullptr) printf("TRAP: %s\n", TRAP_NAME[intIndex]);
    else printf("TRAP: %i\n", intIndex);
    switch (index)
    {
    case TrapIndex::HardFault:
        if (mBase->HFSR.VECTTBL) printf("  %s\n", "Bus fault on vector table read.\n");
        if (mBase->HFSR.FORCED) printf("  %s\n", "Forced hard fault.\n");
        break;
    case TrapIndex::MemManage:
        if (mBase->CFSR.MLSPERR) printf("  Floating point lazy state preservation.\n");
        if (mBase->CFSR.MSTKERR) printf("  Stacking for exception entry fault.\n");
        if (mBase->CFSR.MUNSTKERR) printf("  Unstacking for return from exception fault.\n");
        if (mBase->CFSR.DACCVIOL) printf("  Data access violation.\n");
        if (mBase->CFSR.IACCVIOL) printf("  Instruction access violation.\n");
        if (mBase->CFSR.MMARVALID) printf("  At address %08lx (%lu).\n", mBase->MMFAR, mBase->MMFAR);
        break;
    case TrapIndex::BusFault:
        if (mBase->CFSR.LSPERR) printf("  Floating point lazy state preservation.\n");
        if (mBase->CFSR.STKERR) printf("  Stacking for exception entry fault.\n");
        if (mBase->CFSR.UNSTKERR) printf("  Unstacking for return from exception fault.\n");
        if (mBase->CFSR.IMPRECISERR) printf("  Imprecise data bus error.\n");
        if (mBase->CFSR.IBUSERR) printf("  Instruction bus error.\n");
        if (mBase->CFSR.BFARVALID) printf("  At address %08lx (%lu).\n", mBase->BFAR, mBase->BFAR);
        break;
    case TrapIndex::UsageFault:
        if (mBase->CFSR.DIVBYZERO) printf("  Divide by zero.\n");
        if (mBase->CFSR.UNALIGNED) printf("  Unaligned data access.\n");
        if (mBase->CFSR.NOCP) printf("  FPU is deactivated/not available.\n");
        if (mBase->CFSR.INVPC) printf("  Invalid PC loaded.\n");
        if (mBase->CFSR.INVSTATE) printf("  Invalid state (EPSR).\n");
        if (mBase->CFSR.UNDEFINSTR) printf("  Undefined instruction.\n");
        break;
    default:
        break;
    }

    static const char* REGISTER_NAME[] =
    {
        "R0", "R1", "R2", "R3", "R12", "LR", "PC", "xPSR"
    };

    printf("Stack:\n");

    int i = 0;
    for (const char*& str : REGISTER_NAME)
    {
        printf("  %4s = %08x (%u)\n", str, stackPointer[i], stackPointer[i]);
        ++i;
    }
}
