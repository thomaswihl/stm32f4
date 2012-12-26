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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "ExternalInterrupt.h"
#include "CircularBuffer.h"
#include <cstdint>
#include <queue>
#include <memory>

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
    class Event
    {
    public:
        class Callback
        {
        public:
            virtual void eventCallback(Event* event) = 0;
        };

        Event(Callback& callback) : mCallback(callback) { }

        void callback() { mCallback.eventCallback(this); }

        void setResult(bool success) { mSuccess = success; }
        bool success() { return mSuccess; }
    private:
        bool mSuccess;
        Callback& mCallback;
    };

    enum class TrapIndex
    {
        NMI = 2,
        HardFault = 3,
        MemManage = 4,
        BusFault = 5,
        UsageFault = 6,
        SVCall = 11,
        PendSV = 14,
    };

    typedef unsigned long BaseAddress;

    virtual void handleInterrupt(uint32_t index) = 0;
    virtual void handleTrap(TrapIndex index, unsigned int *stackPointer);
    inline void handleTrap(unsigned int* stackPointer) { handleTrap(static_cast<TrapIndex>(mBase->ICSR.VECTACTIVE), stackPointer); }
    inline void handleInterrupt() { handleInterrupt(mBase->ICSR.VECTACTIVE - 16); }

    virtual void debugRead(char *msg, unsigned int len) = 0;
    virtual void debugWrite(const char *msg, unsigned int len) = 0;
    void printWarning(const char* component, const char* message);
    void printError(const char* component, const char* message);

    static inline System* instance() { return mSystem; }
    static char* increaseHeap(unsigned int incr);
    uint32_t memFree();
    uint32_t memUsed();
    uint32_t stackFree();
    uint32_t stackUsed();

    static void postEvent(Event* event);
    bool waitForEvent(Event*& event);

    template <class T>
    static inline void setRegister(volatile T* reg, uint32_t value) { *reinterpret_cast<volatile uint32_t*>(reg) = value; }

    static inline void sysTick() { ++mTicks; }
    static inline unsigned int ticks() { return mTicks; }
    virtual void usleep(unsigned int us) = 0;
    virtual uint64_t ns() = 0;
    void updateBogoMips();
    uint32_t bogoMips() { return mBogoMips; }

protected:

    System(BaseAddress base);
    ~System();

private:
    struct SCB
    {
        struct __CPUID
        {
            uint32_t Revision : 4;
            uint32_t PartNo : 12;
            uint32_t Constant : 4;
            uint32_t Variant : 4;
            uint32_t Implementer : 8;
        }   CPUID;
        struct __ICSR
        {
            uint32_t VECTACTIVE : 9;
            uint32_t __RESERVED0 : 2;
            uint32_t RETOBASE : 1;
            uint32_t VECTPENDING : 7;
            uint32_t __RESERVED1 : 3;
            uint32_t ISRPENDING : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t PENDSTCLR : 1;
            uint32_t PENDSTSET : 1;
            uint32_t PENDSVCLR : 1;
            uint32_t PENDSVSET : 1;
            uint32_t __RESERVED3 : 2;
            uint32_t NMIPENDSET : 1;
        }   ICSR;
        struct __VTOR
        {
            uint32_t __RESERVED0 : 9;
            uint32_t TBLOFF : 21;
            uint32_t __RESERVED1 : 2;
        }   VTOR;
        struct __AIRCR
        {
            uint32_t VECTRESET : 1;
            uint32_t VECTCLRACTIVE : 1;
            uint32_t SYSRESETREQ : 1;
            uint32_t __RESERVED0 : 5;
            uint32_t PRIGROUP : 3;
            uint32_t __RESERVED1 : 4;
            uint32_t ENDIANESS : 1;
            uint32_t VECTKEYSTAT : 16;
        }   AIRCR;
        struct __SCR
        {
            uint32_t __RESERVED0 : 1;
            uint32_t SLEEPONEXIT : 1;
            uint32_t SLEEPDEEP : 1;
            uint32_t __RESERVED1 : 1;
            uint32_t SEVONPEND : 1;
            uint32_t __RESERVED2 : 27;
        }   SCR;
        struct __CCR
        {
            uint32_t NONBASETHRDENA : 1;
            uint32_t USERSETMPEND : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t UNALIGNTRP : 1;
            uint32_t DIV0TRP : 1;
            uint32_t __RESERVED1 : 3;
            uint32_t BFHFNMIGN : 1;
            uint32_t STKALIGN : 1;
            uint32_t __RESERVED2 : 22;
        }   CCR;
        uint32_t SHPR[3];
        struct __SHCSR
        {
            uint32_t MEMFAULTACT : 1;
            uint32_t BUSFAULTACT : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t USGFAULTACT : 1;
            uint32_t __RESERVED1 : 3;
            uint32_t SVCALLACT : 1;
            uint32_t MONITORACT : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t PENDSVACT : 1;
            uint32_t SYSTICKACT : 1;
            uint32_t USGFAULTPENDED : 1;
            uint32_t MEMFAULTPENDED : 1;
            uint32_t BUSFAULTPENDED : 1;
            uint32_t SVCALLPENDED : 1;
            uint32_t MEMFAULTENA : 1;
            uint32_t BUSFAULTENA : 1;
            uint32_t USGFAULTENA : 1;
            uint32_t __RESERVED3 : 13;
        }   SHCSR;
        struct __CFSR
        {
            uint32_t IACCVIOL : 1;
            uint32_t DACCVIOL : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t MUNSTKERR : 1;
            uint32_t MSTKERR : 1;
            uint32_t MLSPERR : 1;
            uint32_t __RESERVED1 : 1;
            uint32_t MMARVALID : 1;
            uint32_t IBUSERR : 1;
            uint32_t PRECISERR : 1;
            uint32_t IMPRECISERR : 1;
            uint32_t UNSTKERR : 1;
            uint32_t STKERR : 1;
            uint32_t LSPERR : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t BFARVALID : 1;
            uint32_t UNDEFINSTR : 1;
            uint32_t INVSTATE : 1;
            uint32_t INVPC : 1;
            uint32_t NOCP : 1;
            uint32_t __RESERVED3 : 4;
            uint32_t UNALIGNED : 1;
            uint32_t DIVBYZERO : 1;
            uint32_t __RESERVED4 : 6;
        }   CFSR;
        struct __HFSR
        {
            uint32_t __RESERVED0 : 1;
            uint32_t VECTTBL : 1;
            uint32_t __RESERVED1 : 28;
            uint32_t FORCED : 1;
            uint32_t DEBUG_VT : 1;
        }   HFSR;
        uint32_t __RESERVED0;
        uint32_t MMFAR;
        uint32_t BFAR;
        uint32_t AFSR;
    };

    static System* mSystem;
    static char* mHeapEnd;

    static unsigned int mTicks;

    volatile SCB* mBase;
    uint32_t mBogoMips;
    CircularBuffer<Event*> mEventQueue;
};

#endif
