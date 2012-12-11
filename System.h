#ifndef SYSTEM_H
#define SYSTEM_H

#include "ExternalInterrupt.h"

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

    virtual void handleInterrupt(uint32_t index) = 0;
    virtual void handleTrap(uint32_t index);
    virtual int debugWrite(const char* msg, int len) = 0;
    virtual int debugRead(char* msg, int len) = 0;
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

    class Trap : public InterruptController::Handler
    {
    public:
        enum class Index
        {
            NMI = 2,
            HardFault = 3,
            MemManage = 4,
            BusFault = 5,
            UsageFault = 6,
            SVCall = 11,
            PendSV = 14,
            SysTick = 15,
            __COUNT
        };

        Trap(const char* name);
        virtual ~Trap() { }
        virtual void handle(InterruptController::Index index);
    private:
        const char* mName;
    };

    class SysTick : public Trap
    {
    public:
        SysTick(const char* name);
        virtual ~SysTick() { }

        virtual void handle(InterruptController::Index index);
    private:
        uint32_t mTick;
    };

    void setTrap(Trap::Index index, Trap* handler);

private:
    static System* mSystem;
    static char* mHeapEnd;

    Trap mNmi;
    Trap mHardFault;
    Trap mMemManage;
    Trap mBusFault;
    Trap mUsageFault;
    Trap mSVCall;
    Trap mPendSV;
    Trap mSysTick;

    Trap* mTrap[static_cast<unsigned int>(Trap::Index::__COUNT)];
};

#endif
