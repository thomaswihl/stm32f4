#include "ClockControl.h"

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

bool ClockControl::setSystemClock()
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
    int div = mExternalClock / 1000000;
    int mul = 336;
    while (div > 63)
    {
        mul /= 2;
        div /= 2;
    }
    if (div < 2) div = 2;
    while ((mul * div) > 336) --mul;
    if (mul < 2) return false;
    // e.g. for external clock of 8MHz div should be 8 and mul should be 336
    // VCO in = external oscillator / 8 = 1MHz
    mBase->PLLCFGR.PLLM = div;
    // VCO out = VCO in * 336 = 336MHz
    mBase->PLLCFGR.PLLN = mul;
    // PLL out = VCO out / 2 = 168MHz
    mBase->PLLCFGR.PLLP = 0;    // 0 = /2, 1 = /4, 2 = /6, 3 = /8
    // PLL48CLK = VCO out / 7 = 48MHz
    mBase->PLLCFGR.PLLQ = 7;
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
