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
    typedef std::uint32_t BaseAddress;

    virtual void handleInterrupt(uint32_t index) = 0;
    bool tryHandleInterrupt(uint32_t& index);
    static System* instance();
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
