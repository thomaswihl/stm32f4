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
// required for C++
void* __dso_handle;

void __cxa_pure_virtual()
{
    std::printf("Pure fitual function called!\n");
    std::abort();
}

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

    main();

    // calls __fini_array and then calls _fini()
    __libc_fini_array();
    // we should call wfe or wfi but that does bad things to st-link
    while (true)
    {
        //__asm("wfi");
    }
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
    _write(1, "Exiting\n", 8);
    while (1)
    {
        __asm("WFE");
    }
}

}   // extern "C"

System* System::mSystem;
char* System::mHeapEnd;

System* System::instance()
{
    return mSystem;
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

uint32_t System::stackFree()
{
    register char* stack __asm("r0");
    __asm volatile("MOV r0, sp\n" ::: "r0", "cc", "memory");
    return stack - &__stack_start;
}

uint32_t System::stackUsed()
{
    register char* stack __asm("r0");
    __asm volatile("MOV r0, sp\n" ::: "r0", "cc", "memory");
    return &__stack_end - stack;
}

System::System() :
    mNmi("Nmi"),
    mHardFault("HardFault"),
    mMemManage("MemManage"),
    mBusFault("BusFault"),
    mUsageFault("UsageFault"),
    mSVCall("SVCall"),
    mDebugMonitor("DebugMonitor"),
    mPendSV("PendSV"),
    mSysTick("SysTick")
{
    // Make sure we are the first and only instance
    assert(mSystem == 0);
    mSystem = this;
    setTrap(Trap::NMI, &mNmi);
    setTrap(Trap::HardFault, &mHardFault);
    setTrap(Trap::MemManage, &mMemManage);
    setTrap(Trap::BusFault, &mBusFault);
    setTrap(Trap::UsageFault, &mUsageFault);
    setTrap(Trap::SVCall, &mSVCall);
    setTrap(Trap::DebugMonitor, &mDebugMonitor);
    setTrap(Trap::PendSV, &mPendSV);
    setTrap(Trap::SysTick, &mSysTick);
}

System::~System()
{
}

bool System::tryHandleInterrupt(uint32_t &index)
{
    if (index < static_cast<unsigned int>(Trap::__COUNT))
    {
        // It's a trap so handle it
        if (mTrap[index] != 0) mTrap[index]->handle(index);
    }
    else
    {
        // It's an interrupt, we can't handle it
        index -= static_cast<uint32_t>(Trap::__COUNT);
        return false;
    }
    return true;
}

void System::setTrap(Trap::TrapIndex index, Trap *handler)
{
    mTrap[static_cast<int>(index)] = handler;
}

System::Trap::Trap(const char *name) : mName(name)
{
}

void System::Trap::handle(Interrupt::Index index)
{
    std::printf("TRAP: %s(%i)\n", mName, index);
}


System::Buffer::Buffer(size_t size) :
    mSize(size),
    mData(new char[size]),
    mDelete(true)
{
}

System::Buffer::Buffer(char *data, std::size_t size) :
    mSize(size),
    mData(data),
    mDelete(false)
{
}

System::Buffer::Buffer(const char *data, std::size_t size) :
    mSize(size),
    mData(const_cast<char*>(data)),
    mDelete(false)
{
}

System::Buffer::~Buffer()
{
    if (mDelete) delete mData;
}

