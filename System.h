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
        enum class Component { Invalid, Serial };

        Event() : mComponent(Component::Invalid) { }
        Event(Component component) : mComponent(component) { }
        Component component() { return mComponent; }
    private:
        Component mComponent;
    };
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

    void postEvent(std::shared_ptr<Event> event);
    std::shared_ptr<Event> waitForEvent();

    template <class T>
    static inline void setRegister(volatile T* reg, uint32_t value) { *reinterpret_cast<volatile uint32_t*>(reg) = value; }

    static inline void sysTick() { ++mSysTick; }
    static inline unsigned int ticks() { return mSysTick; }
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

    Trap* mTrap[static_cast<unsigned int>(Trap::Index::__COUNT)];

    std::queue<std::shared_ptr<Event>> mEventQueue;
    static unsigned int mSysTick;
};

#endif
