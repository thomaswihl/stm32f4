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
    // save the sp and lr (containing return info)
    __asm("mov r0, sp");
    __asm("push {r4, r5, r6, r7, r8, r9, r10, r11}");
    __asm("push {r0, lr}");
    //  __asm("add.w r0, r0, #8");
    __asm("bl Trap2");
    __asm("pop {r4, r5, r6, r7, r8, r9, r10, r11}");
    __asm("pop {r0, lr}");
    __asm("mov sp, r0");
    while (true) ;
    // return from fault handler (doesn't work for whatever reason)
    __asm("bx lr");
}

void Trap2(unsigned int* stackPointer)
{
    System::instance()->handleTrap(stackPointer);
    // replace the previous pc with the new one, as it doesn't make sense to return to the faulty instruction.
    // We have to mask the lowest bit (indicating thumb code)
    stackPointer[8] = reinterpret_cast<unsigned int>(&_exit);
}

void __attribute__((interrupt)) Isr()
{
    System::instance()->handleInterrupt();
}

void __attribute__((interrupt)) SysTick()
{
    System::instance()->handleSysTick();
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
    System::initStack();
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
    return len;
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
    System::instance()->debugWrite(ptr, len);
    return len;
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
    printf("EXIT(%i)\n", v);
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
const unsigned int System::STACK_MAGIC;

void System::initStack()
{
    unsigned int* p = reinterpret_cast<unsigned int*>(&__stack_start);
    register unsigned int* stackPointer __asm("sp");
    for (; p < stackPointer; ++p)
    {
        *p = STACK_MAGIC;
    }
}


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

uint32_t System::memDataUsed()
{
    return &__data_end - &__data_start;
}

uint32_t System::memBssUsed()
{
    return &__bss_end - &__bss_start;
}

uint32_t System::stackFree()
{
    register char* stack __asm("sp");
    return stack - &__stack_start;
}

uint32_t System::stackUsed()
{
    register char* stack __asm("sp");
    return &__stack_end - stack;
}

uint32_t System::stackMaxUsed()
{
    unsigned int* p = reinterpret_cast<unsigned int*>(&__stack_start);
    register unsigned int* stackPointer __asm("sp");
    for (; p < stackPointer; ++p)
    {
        if (*p != STACK_MAGIC) break;
    }
    return (reinterpret_cast<unsigned int*>(&__stack_end) - p) * sizeof(unsigned int);
}

void System::postEvent(Event *event)
{
    mSystem->mEventQueue.push(event);
}

bool System::waitForEvent(Event *&event)
{
    while (mEventQueue.used() == 0)
    {
        __asm("wfi");
    }
    return mEventQueue.pop(event);
}

void System::updateBogoMips()
{
    uint64_t start = ns();
    for (unsigned int i = 100000; i != 0; --i)
    {
        __asm("");
    }
    uint64_t end = ns();
    mBogoMips = 100000000000000ul / (end - start);
}

System::System(BaseAddress base) :
    mBase(reinterpret_cast<volatile SCB*>(base)),
    mBogoMips(0),
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
    if (intIndex < 16 && TRAP_NAME[intIndex] != nullptr) printf("\n\nTRAP: %s\n", TRAP_NAME[intIndex]);
    else printf("\n\nTRAP: %i\n", intIndex);
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

    struct Register
    {
        const char* const name;
        int offset;
    };

    static const Register REGISTER[] =
    {
        {"R0", 0},
        {"R1", 1},
        {"R2", 2},
        {"R3", 3},
        {"R4", -8},
        {"R5", -7},
        {"R6", -6},
        {"R7", -5},
        {"R8", -4},
        {"R9", -3},
        {"R10", -2},
        {"R11", -1},
        {"R12", 4},
        {"LR", 5},
        {"PC", 6},
        {"xPSR", 7},
    };

    printf("Stack (0x%08x):\n", reinterpret_cast<unsigned int>(stackPointer));

    int i = 0;
    for (const Register& reg : REGISTER)
    {
        printf("  %4s = 0x%08x (%u)\n", reg.name, stackPointer[reg.offset], stackPointer[reg.offset]);
        ++i;
    }
}

void System::printWarning(const char *component, const char *message)
{
    printf("\nWARNING in %s: %s\n", component, message);
}

void System::printError(const char *component, const char *message)
{
    printf("\nERROR in %s: %s\n", component, message);
}

