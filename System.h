#ifndef SYSTEM_H
#define SYSTEM_H

#include "Interrupt.h"

#include <cstdint>
extern "C"
{
// Linker symbols
extern char __stack_start;
extern char __stack_end;
extern char __bss_start;
extern char __bss_end;
extern char __data_start;
extern char __data_end;
extern const char __data_rom_start;
extern char __heap_start;
extern char __heap_end;

extern void _start();
}

class System
{
public:
    typedef unsigned long BaseAddress;

    class Buffer
    {
    public:
        Buffer(std::size_t size);
        Buffer(char* data, std::size_t size);
        Buffer(const char* data, std::size_t size);
        ~Buffer();

        std::size_t size() { return mSize; }
        char* data() { return mData; }
    private:
        std::size_t mSize;
        char* mData;
        bool mDelete;
    };

    virtual void handleInterrupt(uint32_t index) = 0;
    virtual void handleTrap(uint32_t index);
    virtual void debugWrite(const char* msg, int len) = 0;
    virtual void debugRead(char* msg, int len) = 0;
    static inline System* instance() { return mSystem; }
    static char* increaseHeap(unsigned int incr);
    uint32_t memFree();
    uint32_t memUsed();
    uint32_t stackFree();
    uint32_t stackUsed();

    template <class T>
    static inline void setRegister(volatile T* reg, uint32_t value) { *reinterpret_cast<volatile uint32_t*>(reg) = value; }
protected:

    System();
    ~System();

    class Trap : public Interrupt::Handler
    {
    public:
        enum TrapIndex
        {
            NMI = 2,
            HardFault = 3,
            MemManage = 4,
            BusFault = 5,
            UsageFault = 6,
            SVCall = 11,
            DebugMonitor = 12,
            PendSV = 14,
            SysTick = 15,
            __COUNT
        };

        Trap(const char* name);
        virtual ~Trap() { }
        virtual void handle(Interrupt::Index index);
    private:
        const char* mName;
    };


    void setTrap(Trap::TrapIndex index, Trap* handler);

private:
    static System* mSystem;
    static char* mHeapEnd;

    Trap mNmi;
    Trap mHardFault;
    Trap mMemManage;
    Trap mBusFault;
    Trap mUsageFault;
    Trap mSVCall;
    Trap mDebugMonitor;
    Trap mPendSV;
    Trap mSysTick;

    Trap* mTrap[Trap::__COUNT];
};

#endif
