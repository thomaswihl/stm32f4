#ifndef SYSTICKCONTROL_H
#define SYSTICKCONTROL_H

#include "System.h"
#include "ClockControl.h"

class SysTickControl : ClockControl::ChangeHandler
{
public:
    SysTickControl(System::BaseAddress base, ClockControl* clock, unsigned int msInterval);
    void enable();
    void disable();

    void usleep(unsigned int us);

protected:
    virtual void clockPrepareChange(uint32_t newClock);
    virtual void clockChanged(uint32_t newClock);

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
    unsigned int mInterval;
};

#endif // SYSTICKCONTROL_H
