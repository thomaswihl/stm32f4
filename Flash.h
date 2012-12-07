#ifndef FLASH_H
#define FLASH_H

#include "ClockControl.h"

class Flash : public ClockControl::ChangeHandler
{
public:
    enum class Feature
    {
        InstructionCache,
        DataCache,
        Prefetch,
    };

    Flash(System::BaseAddress base, ClockControl& clockControl);
    virtual ~Flash();
    void set(Feature feature, bool enable);
protected:
    virtual void clockPrepareChange(uint32_t newClock);
    virtual void clockChanged(uint32_t newClock);

private:
    struct FLASH
    {
        struct __ACR
        {
            uint16_t LATENCY : 3;
            uint16_t __RESERVED0 : 5;
            uint16_t PRFTEN : 1;
            uint16_t ICEN : 1;
            uint16_t DCEN : 1;
            uint16_t ICRST : 1;
            uint16_t DCRST : 1;
            uint16_t __RESERVED1 : 3;
            uint16_t __RESERVED2;
        }   ACR;
    };
    volatile FLASH* mBase;

    uint16_t getWaitStates(uint32_t clock);
};

#endif // FLASH_H
