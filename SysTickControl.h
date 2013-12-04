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

#ifndef SYSTICKCONTROL_H
#define SYSTICKCONTROL_H

#include "System.h"
#include "ClockControl.h"
#include "InterruptController.h"

class SysTickControl : ClockControl::Callback
{
public:
    class RepeatingEvent : public System::Event
    {
    public:
        RepeatingEvent(Callback& callback, int ms) : System::Event(callback), mMs(ms), mMsFromStart(0)
        { }

        void millisecondsPassed(unsigned ms);
        unsigned ms() const { return mMs; }
        unsigned msRemain() const { return mMs - mMsFromStart; }
    private:
        unsigned mMs;

        unsigned mMsFromStart;
    };

    SysTickControl(System::BaseAddress base, ClockControl* clock);
    ~SysTickControl() { disable(); }

    void addRepeatingEvent(RepeatingEvent* event);
    void removeRepeatingEvent(RepeatingEvent* event);

    void tick();

    void usleep(unsigned int us);
    uint64_t ns();

protected:
    virtual void clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock);

private:
    struct STK
    {
        struct __CTRL
        {
            uint32_t ENABLE : 1;
            uint32_t TICKINT : 1;
            uint32_t CLKSOURCE : 1;
            uint32_t __RESERVED0 : 13;
            uint32_t COUNTFLAG : 1;
            uint32_t __RESERVED1 : 15;
        }   CTRL;
        uint32_t RELOAD;
        uint32_t VAL;
        struct __CALIB
        {
            uint32_t TENMS : 24;
            uint32_t __RESERVED0 : 6;
            uint32_t SKEW : 1;
            uint32_t NOREF : 1;
        }   CALIB;
    };
    volatile STK* mBase;
    ClockControl* mClock;
    unsigned mSingleCountTime;
    uint32_t mCountPerMs;
    unsigned mMilliseconds;
    unsigned mNextTick;
    std::vector<RepeatingEvent*> mRepeatingEvents;

    void config();
    void enable();
    void disable();
    void setNextTick(unsigned ms);
};

#endif // SYSTICKCONTROL_H
