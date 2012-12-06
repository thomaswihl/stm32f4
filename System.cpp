#include "System.h"

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>

extern "C"
{
// required for C++
void* __dso_handle;

void __cxa_pure_virtual()
{
    std::printf("Pure fitual function called!\n");
    std::abort();
}

void *operator new(std::size_t size)
{
   return malloc(size);
}

void *operator new[](std::size_t size)
{
   return ::operator new(size);
}

void operator delete(void *mem)
{
   free(mem);
}

void operator delete[](void *mem)
{
   ::operator delete(mem);
}


extern void __libc_init_array();
extern void __libc_fini_array();
extern int main();

// our entry point after reset
void _start()
{
    memcpy(&__data_start, &__data_rom_start, &__data_end - &__data_start);
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
    // calls __preinit_array, call _init() and then calls __init_array (constructors)
    __libc_init_array();

    // Make sure we have one instance of our System class
    assert(System::instance() != 0);

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
}

void _fini()
{
}

}   // extern "C"

System* System::mSystem;

//uint32_t System::getCoreClock()
//{
//    using namespace StmLib;
//    uint32_t source = RCC->CFGR & RCC_CFGR_SWS;

//    switch (source)
//    {
//    case 0x00:  /* HSI used as system clock source */
//        return HSI_VALUE;
//    case 0x04:  /* HSE used as system clock source */
//        return HSE_VALUE;
//    case 0x08:  /* PLL used as system clock source */
//        return getCoreClockPll();
//    default:
//        return HSI_VALUE;
//    }
//}

System* System::instance()
{
    return mSystem;
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
    // Make sure we are the first and only instance
    assert(mSystem == 0);
    mSystem = this;
    setTrap(Trap::NMI, &mNmi);
    setTrap(Trap::HardFault, &mHardFault);
    setTrap(Trap::MemManage, &mMemManage);
    setTrap(Trap::BusFault, &mBusFault);
    setTrap(Trap::UsageFault, &mUsageFault);
    setTrap(Trap::SVCall, &mSVCall);
    setTrap(Trap::DebugMonitor, &mDebugMonitor);
    setTrap(Trap::PendSV, &mPendSV);
    setTrap(Trap::SysTick, &mSysTick);
}

System::~System()
{
}

bool System::tryHandleInterrupt(uint32_t &index)
{
    if (index < static_cast<unsigned int>(Trap::__COUNT))
    {
        // It's a trap so handle it
        if (mTrap[index] != 0) mTrap[index]->handle(index);
    }
    else
    {
        // It's an interrupt, we can't handle it
        index -= static_cast<uint32_t>(Trap::__COUNT);
        return false;
    }
    return true;
}

void System::setTrap(Trap::TrapIndex index, Trap *handler)
{
    mTrap[static_cast<int>(index)] = handler;
}

//uint32_t System::getCoreClockPll()
//{
//    static const uint8_t AHB_PRESCALER_TABLE[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
//    using namespace StmLib;

//    uint32_t pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
//    uint32_t pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    
//    uint32_t pllvco;
//    if (pllsource != 0) pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
//    else pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);

//    uint32_t pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
//    uint32_t clock = pllvco / pllp;

//    clock >>= AHB_PRESCALER_TABLE[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
//    return clock;
//}

//void System::initUart()
//{
//    using namespace StmLib;
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

//    /* GPIOA Configuration:  USART2 TX on PA2 */
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    /* Connect USART2 pins to AF2 */
//    // TX = PA2, RX = PA3
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

//    USART_InitStructure.USART_BaudRate = 115200;
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_Init(USART2, &USART_InitStructure);

//    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		 // we want to configure the USART2 interrupts
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART2 interrupts
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART2 interrupts are globally enabled
//    NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

//    USART_Cmd(USART2, ENABLE); // enable USART2

//}

//void System::setSysClock()
//{
//    using namespace StmLib;
//    /******************************************************************************/
//    /*            PLL (clocked by HSE) used as System clock source                */
//    /******************************************************************************/

//    /* Enable HSE */
//    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

//    /* Wait till HSE is ready and if Time out is reached exit */
//    bool ready;
//    int counter = 0;
//    do
//    {
//        ready = RCC->CR & RCC_CR_HSERDY;
//        counter++;
//    } while (!ready && (counter != HSE_STARTUP_TIMEOUT));

//    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
//    {
//        /* Enable high performance mode, System frequency up to 168 MHz */
//        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//        PWR->CR |= PWR_CR_PMODE;

//        /* HCLK = SYSCLK / 1*/
//        RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

//        /* PCLK2 = HCLK / 2*/
//        RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

//        /* PCLK1 = HCLK / 4*/
//        RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

//        /* Configure the main PLL */
//        RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
//                (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

//        /* Enable the main PLL */
//        RCC->CR |= RCC_CR_PLLON;

//        /* Wait till the main PLL is ready */
//        while((RCC->CR & RCC_CR_PLLRDY) == 0)
//        {
//        }

//        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
//        FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

//        /* Select the main PLL as system clock source */
//        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
//        RCC->CFGR |= RCC_CFGR_SW_PLL;

//        /* Wait till the main PLL is used as system clock source */
//        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
//        {
//        }
//    }
//    else
//    { /* If HSE fails to start-up, the application will have wrong clock
//        configuration. User can add here some code to deal with this error */
//    }

//}


System::Trap::Trap(const char *name) : mName(name)
{
}

void System::Trap::handle(Interrupt::Index index)
{
    std::printf("TRAP: %s(%i)\n", mName, index);
}


