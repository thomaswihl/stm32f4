#include "System.h"


namespace StmLib
{
#define HSE_VALUE    ((uint32_t)8000000)
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_rcc.h>
#include <misc.h>
#include <stm32f4xx_exti.h>
#undef SysTick
}

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>



extern "C"
{
// This is the init stuff and bringup code
extern char __stack_start;
extern char __stack_end;
extern char __bss_start;
extern char __bss_end;
extern char __data_start;
extern char __data_end;
extern const char __data_rom_start;
extern char __heap_start;
extern char __heap_end;
extern void __libc_init_array();
extern void __libc_fini_array();
extern void _start();
extern int main();
void* __dso_handle;

void __attribute__((interrupt)) Isr()
{
    register int index __asm("r0");
    __asm volatile("MRS r0, IPSR\n" ::: "r0", "cc", "memory");
    printf("Interrupt: %i\n", index & 0xff);
    if (!System::instance()->handleIsr(index))
    {
        printf("Unhandled interrupt: %i\n", index & 0xff);
    }
}

void __cxa_pure_virtual()
{
    std::printf("Pure fitual function called!\n");
    std::abort();
}

void *operator new(std::size_t size)
{
   return malloc(size);
}

void operator delete(void *mem)
{
   free(mem);
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


void _start()
{
    memcpy(&__data_start, &__data_rom_start, &__data_end - &__data_start);
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
    // calls __preinit_array, call _init() which calls the System::init() and then calls __init_array
    __libc_init_array();

    System* system = System::instance();
    assert(system != 0);
    system->init();

    main();

    // calls __fini_array and then calls _fini()
    __libc_fini_array();
    // we should call wfe or wfi but that does bad things to st-link
    while (true)
    {
        //__asm("wfi");
    }
}

void _init()
{
    System::preInit();
}

void _fini()
{

}

}   // extern "C"

#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
    This value must be a multiple of 0x200. */


/* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
#define PLL_M      8
#define PLL_N      336

/* SYSCLK = PLL_VCO / PLL_P */
#define PLL_P      2

/* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
#define PLL_Q      7

System* System::mSystem;

void System::preInit()
{
    using namespace StmLib;
    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x24003010;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    /* Configure the System clock source, PLL Multiplier and Divider factors,
    AHB/APBx prescalers and Flash settings ----------------------------------*/
    setSysClock();

    initUart();

    /* Configure the Vector Table location add offset address ------------------*/
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
}

void System::init()
{
}


uint32_t System::getCoreClock()
{
    using namespace StmLib;
    uint32_t source = RCC->CFGR & RCC_CFGR_SWS;

    switch (source)
    {
    case 0x00:  /* HSI used as system clock source */
        return HSI_VALUE;
    case 0x04:  /* HSE used as system clock source */
        return HSE_VALUE;
    case 0x08:  /* PLL used as system clock source */
        return getCoreClockPll();
    default:
        return HSI_VALUE;
    }
}

bool System::handleIsr(unsigned int index) const
{
    if (index < static_cast<unsigned int>(TrapIndex::__COUNT))
    {
        if (mTrap[index] == 0) return false;
        mTrap[index]->irq(index);
    }
    else
    {
        index -= static_cast<unsigned int>(TrapIndex::__COUNT);
        if (mInterrupt[index] == 0) return false;
        mInterrupt[index]->irq(index);
    }
    return true;
}

System* System::instance()
{
    return mSystem;
}

void System::setInterrupt(System::InterruptIndex index, System::IrqHandler *handler)
{
    mInterrupt[static_cast<int>(index)] = handler;
}

System::System() :
    mNmi("Nmi"),
    mHardFault("HardFault"),
    mMemManage("MemManage"),
    mBusFault("BusFault"),
    mUsageFault("UsageFault"),
    mSVCall("SVCall"),
    mDebugMonitor("DebugMonitor"),
    mPendSV("PendSV"),
    mSysTick("SysTick")
{
    assert(mSystem == 0);
    mSystem = this;
    setTrap(TrapIndex::NMI, &mNmi);
    setTrap(TrapIndex::HardFault, &mHardFault);
    setTrap(TrapIndex::MemManage, &mMemManage);
    setTrap(TrapIndex::BusFault, &mBusFault);
    setTrap(TrapIndex::UsageFault, &mUsageFault);
    setTrap(TrapIndex::SVCall, &mSVCall);
    setTrap(TrapIndex::DebugMonitor, &mDebugMonitor);
    setTrap(TrapIndex::PendSV, &mPendSV);
    setTrap(TrapIndex::SysTick, &mSysTick);
}

System::~System()
{
}

void System::setTrap(System::TrapIndex index, System::IrqHandler *handler)
{
    mTrap[static_cast<int>(index)] = handler;
}

uint32_t System::getCoreClockPll()
{
    static const uint8_t AHB_PRESCALER_TABLE[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
    using namespace StmLib;

    uint32_t pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
    uint32_t pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    
    uint32_t pllvco;
    if (pllsource != 0) pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
    else pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);

    uint32_t pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
    uint32_t clock = pllvco / pllp;

    clock >>= AHB_PRESCALER_TABLE[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    return clock;
}

void System::initUart()
{
    using namespace StmLib;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* GPIOA Configuration:  USART2 TX on PA2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Connect USART2 pins to AF2 */
    // TX = PA2, RX = PA3
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		 // we want to configure the USART2 interrupts
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART2 interrupts
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART2 interrupts are globally enabled
    NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

    USART_Cmd(USART2, ENABLE); // enable USART2

}

void System::setSysClock()
{
    using namespace StmLib;
    /******************************************************************************/
    /*            PLL (clocked by HSE) used as System clock source                */
    /******************************************************************************/

    /* Enable HSE */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait till HSE is ready and if Time out is reached exit */
    bool ready;
    int counter = 0;
    do
    {
        ready = RCC->CR & RCC_CR_HSERDY;
        counter++;
    } while (!ready && (counter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
    {
        /* Enable high performance mode, System frequency up to 168 MHz */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_PMODE;

        /* HCLK = SYSCLK / 1*/
        RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

        /* PCLK2 = HCLK / 2*/
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

        /* PCLK1 = HCLK / 4*/
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

        /* Configure the main PLL */
        RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
                (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

        /* Enable the main PLL */
        RCC->CR |= RCC_CR_PLLON;

        /* Wait till the main PLL is ready */
        while((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
        }

        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
        FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

        /* Select the main PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= RCC_CFGR_SW_PLL;

        /* Wait till the main PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock
        configuration. User can add here some code to deal with this error */
    }

}


System::Trap::Trap(const char *name) : mName(name)
{
}

void System::Trap::irq(int index)
{
    std::printf("TRAP: %s(%i)\n", mName, index);
}
