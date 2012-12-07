#include "ClockControl.h"

#include <cstdlib>

ClockControl::ClockControl(System::BaseAddress base, uint32_t externalClock) :
    mBase(reinterpret_cast<volatile RCC*>(base)),
    mExternalClock(externalClock)
{
    // enable internal clock
    mBase->CR.HSION = 1;

//    /* Reset CFGR register */
//    RCC->CFGR = 0x00000000;

//    /* Reset HSEON, CSSON and PLLON bits */
//    RCC->CR &= (uint32_t)0xFEF6FFFF;

//    /* Reset PLLCFGR register */
//    RCC->PLLCFGR = 0x24003010;

//    /* Reset HSEBYP bit */
//    RCC->CR &= (uint32_t)0xFFFBFFFF;

//    /* Disable all interrupts */
//    RCC->CIR = 0x00000000;

//    /* Configure the System clock source, PLL Multiplier and Divider factors,
//        AHB/APBx prescalers and Flash settings ----------------------------------*/
//    setSysClock();

//    /* Configure the Vector Table location add offset address ------------------*/
//    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
}

ClockControl::~ClockControl()
{
}

bool ClockControl::setSystemClock(uint32_t clock)
{
    // enable external oscillator
    mBase->CR.HSEON = 1;

    int timeout = 0x1000;
    while (!mBase->CR.HSERDY)
    {
        if (--timeout == 0)
        {
            // external oscillator not working, disable and return
            mBase->CR.HSEON = 0;
            return false;
        }
    }
    mBase->APB1ENR.PWREN = 1;
    // AHB = system clock
    mBase->CFGR.HPRE = 0;   // 0-7 = /1, 8 = /2, 9 = /4, 10 = /8, 11 = /16 ... 15 = /512
    // APB2 = AHB/2
    mBase->CFGR.PPRE2 = 4;  // 0-3 = /1, 4 = /2, 5 = /4, 6 = /8, 7 = /16
    // APB1 = AHB / 4
    mBase->CFGR.PPRE1 = 5;  // 0-3 = /1, 4 = /2, 5 = /4, 6 = /8, 7 = /16
    uint32_t div, mul;
    if (!getPllConfig(clock * 2, div, mul)) return false;
    // e.g. for external clock of 8MHz div should be 8 and mul should be 336
    // VCO in = external oscillator / 8 = 1MHz
    mBase->PLLCFGR.PLLM = div;  // 2..63
    // VCO out = VCO in * 336 = 336MHz
    mBase->PLLCFGR.PLLN = mul;  // 192..432
    // PLL out = VCO out / 2 = 168MHz
    mBase->PLLCFGR.PLLP = 0;    // 0 = /2, 1 = /4, 2 = /6, 3 = /8
    // PLL48CLK = VCO out / 7 = 48MHz
    mBase->PLLCFGR.PLLQ = 7;    // 2..15
    // external oscillator is source for PLL
    mBase->PLLCFGR.PLLSRC = 1;
    // enable PLL and wait till it is ready
    mBase->CR.PLLON = 1;
    while (!mBase->CR.PLLRDY)
    {
    }
    // configure PLL as clock source and wait till it is active
    mBase->CFGR.SW = 2;    // 0 = internal, 1 = external, 2 = PLL
    while (mBase->CFGR.SWS != 2)
    {
    }

    return true;
}

bool ClockControl::getPllConfig(uint32_t clock, uint32_t &div, uint32_t &mul)
{
    // we want to do a 32bit calculation, so we divide pll and external clock by 10
    uint32_t external = mExternalClock / 10;
    uint32_t pll = clock / 10;
    static const uint32_t vcoInMin = 1 * 1000 * 1000 / 10;
    static const uint32_t vcoInMax = 2 * 1000 * 1000 / 10;
    uint32_t tmpdiv, tmpmul, bestdelta = pll;
    for (tmpdiv = 2; tmpdiv <= 63; ++tmpdiv)
    {
        uint32_t vcoIn = external / tmpdiv;
        if (vcoIn >= vcoInMin && vcoIn <= vcoInMax)
        {
            // PLL = external / div * mul;
            tmpmul = pll * tmpdiv / external;
            if (tmpmul >= 192 && tmpmul <= 432)
            {
                uint32_t tmppll = external / tmpdiv * tmpmul;
                uint32_t delta = std::abs(static_cast<int>(pll) - static_cast<int>(tmppll));
                if (delta < bestdelta)
                {
                    div = tmpdiv;
                    mul = tmpmul;
                    bestdelta = delta;
                    if (delta == 0) return true;
                }
            }
        }
    }
    // we didn't find an exact match, if we found an aproximate one return it
    if (bestdelta != pll) return true;
    // we didn't find anything, set div and mul to the lowest cock possible
    div = 63;
    mul = 192;
    return false;
}
