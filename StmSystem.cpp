#include "StmSystem.h"

#include <cstdio>

#define HSE_VALUE    ((uint32_t)8000000)

extern "C"
{
void __attribute__((interrupt)) Isr()
{
    register int index __asm("r0");
    __asm volatile("MRS r0, IPSR\n" ::: "r0", "cc", "memory");
    std::printf("Interrupt: %i\n", index & 0xff);
    System::instance()->handleInterrupt(index & 0xff);
}

extern void (* const gIsrVectorTable[])(void);
__attribute__ ((section(".isr_vector_table")))
void (* const gIsrVectorTable[])(void) = {
    // 16 trap functions for ARM
    (void (* const)())&__stack_end, (void (* const)())&_start, Isr, Isr, Isr, Isr, 0, 0,
    0, 0, Isr, Isr, 0, Isr, Isr,
    // 82 hardware interrupts specific to the STM32F407
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr
};

}

#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field.
    This value must be a multiple of 0x200. */


///* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
//#define PLL_M      8
//#define PLL_N      336

///* SYSCLK = PLL_VCO / PLL_P */
//#define PLL_P      2

///* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
//#define PLL_Q      7


StmSystem::StmSystem() :
    mClock(static_cast<System::BaseAddress>(BaseAddress::RCC), 8000000),
    mInt(static_cast<System::BaseAddress>(BaseAddress::EXTI), 82),
    mDebug(static_cast<System::BaseAddress>(BaseAddress::USART2))
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::init()
{
}

void StmSystem::handleInterrupt(uint32_t index)
{
    if (!tryHandleInterrupt(index)) mInt.handle(index);
}
