#include "Flash.h"

Flash::Flash(System::BaseAddress base, ClockControl &clockControl) :
    mBase(reinterpret_cast<volatile FLASH*>(base))
{
    clockControl.addChangeHandler(this);
}

Flash::~Flash()
{
}

void Flash::set(Flash::Feature feature, bool enable)
{
    switch (feature)
    {
    case Feature::InstructionCache: mBase->ACR.ICEN = enable; break;
    case Feature::DataCache: mBase->ACR.DCEN = enable; break;
    case Feature::Prefetch: mBase->ACR.PRFTEN = enable; break;
    }
}

void Flash::clockPrepareChange(uint32_t newClock)
{
    uint16_t ws = getWaitStates(newClock);
    if (mBase->ACR.LATENCY < ws) mBase->ACR.LATENCY = ws;
}

void Flash::clockChanged(uint32_t newClock)
{
    uint16_t ws = getWaitStates(newClock);
    if (mBase->ACR.LATENCY > ws) mBase->ACR.LATENCY = ws;
}

uint16_t Flash::getWaitStates(uint32_t clock)
{
    if (clock <= 30000000) return 0;
    if (clock <= 60000000) return 1;
    if (clock <= 90000000) return 2;
    if (clock <= 120000000) return 3;
    if (clock <= 150000000) return 4;
    if (clock <= 168000000) return 5;
    return 6;
}
