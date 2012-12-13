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
void __attribute__((interrupt)) Trap()
{
    register int index __asm("r12");
    __asm volatile("mrs r12, IPSR");
    System::instance()->handleTrap(index & 0xff);
}

void __attribute__((interrupt)) SysTick()
{
    System::sysTick();
}

void __attribute__((interrupt)) Isr()
{
    register int index __asm("r12");
    __asm volatile("mrs r12, IPSR");
    System::instance()->handleInterrupt((index & 0xff) - 16);
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
    _write(1, "Exiting\n", 8);
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

void System::postEvent(std::shared_ptr<Event> event)
{
    mEventQueue.push(event);
}

std::shared_ptr<System::Event> System::waitForEvent()
{
    while (mEventQueue.size() == 0)
    {
        __asm("wfi");
    }
    std::shared_ptr<System::Event> event = mEventQueue.back();
    mEventQueue.pop();
    return event;
}

System::System() :
    mNmi("Nmi"),
    mHardFault("HardFault"),
    mMemManage("MemManage"),
    mBusFault("BusFault"),
    mUsageFault("UsageFault"),
    mSVCall("SVCall"),
    mPendSV("PendSV")
{
    // Make sure we are the first and only instance
    assert(mSystem == 0);
    mSystem = this;
    setTrap(Trap::Index::NMI, &mNmi);
    setTrap(Trap::Index::HardFault, &mHardFault);
    setTrap(Trap::Index::MemManage, &mMemManage);
    setTrap(Trap::Index::BusFault, &mBusFault);
    setTrap(Trap::Index::UsageFault, &mUsageFault);
    setTrap(Trap::Index::SVCall, &mSVCall);
    setTrap(Trap::Index::PendSV, &mPendSV);
}

System::~System()
{
}

// The stack looks like this: (FPSCR, S15-S0) xPSR, PC, LR, R12, R3, R2, R1, R0
// With SP at R0 and (FPSCR, S15-S0) being optional
void System::handleTrap(uint32_t index)
{
    if (index < static_cast<unsigned int>(Trap::Index::__COUNT) && mTrap[index] != 0) mTrap[index]->handle(index);
    _write(1, "Unhandled trap\n", 15);
    while (true)
    {
        __asm("wfi");
    }
}

void System::setTrap(Trap::Index index, Trap *handler)
{
    mTrap[static_cast<int>(index)] = handler;
}

System::Trap::Trap(const char *name) : mName(name)
{
}

void System::Trap::handle(InterruptController::Index index)
{
    char c;
    _write(1, "TRAP: ", 6);
    c = '0' + index / 10;
    _write(1, &c, 1);
    c = '0' + index % 10;
    _write(1, &c, 1);
    _write(1, "\n", 1);
    while (true)
    {
        __asm("wfi");
    }
}

System::SysTick::SysTick(const char *name) :
    Trap(name),
    mTick(0)
{
}

void System::SysTick::handle(InterruptController::Index index)
{
    ++mTick;
}
