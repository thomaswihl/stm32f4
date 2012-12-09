#include "StmSystem.h"

#include <cstdio>

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

StmSystem::StmSystem() :
    mGpioA(static_cast<System::BaseAddress>(BaseAddress::GPIOA)),
    mGpioB(static_cast<System::BaseAddress>(BaseAddress::GPIOB)),
    mGpioC(static_cast<System::BaseAddress>(BaseAddress::GPIOC)),
    mGpioD(static_cast<System::BaseAddress>(BaseAddress::GPIOD)),
    mGpioE(static_cast<System::BaseAddress>(BaseAddress::GPIOE)),
    mGpioF(static_cast<System::BaseAddress>(BaseAddress::GPIOF)),
    mGpioG(static_cast<System::BaseAddress>(BaseAddress::GPIOG)),
    mGpioH(static_cast<System::BaseAddress>(BaseAddress::GPIOH)),
    mGpioI(static_cast<System::BaseAddress>(BaseAddress::GPIOI)),
    mClock(static_cast<System::BaseAddress>(BaseAddress::RCC), 8000000),
    mInt(static_cast<System::BaseAddress>(BaseAddress::EXTI), 82),
    mUsart1(static_cast<System::BaseAddress>(BaseAddress::USART1), &mClock, ClockControl::Clock::APB2),
    mUsart2(static_cast<System::BaseAddress>(BaseAddress::USART2), &mClock, ClockControl::Clock::APB1),
    mUsart3(static_cast<System::BaseAddress>(BaseAddress::USART3), &mClock, ClockControl::Clock::APB1),
    mUart4(static_cast<System::BaseAddress>(BaseAddress::UART4), &mClock, ClockControl::Clock::APB1),
    mUart5(static_cast<System::BaseAddress>(BaseAddress::UART5), &mClock, ClockControl::Clock::APB1),
    mUsart6(static_cast<System::BaseAddress>(BaseAddress::USART6), &mClock, ClockControl::Clock::APB2),
    mDebug(mUsart2),
    mFlash(static_cast<System::BaseAddress>(BaseAddress::FLASH), mClock)
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::init()
{
    mClock.enable(ClockControl::Function::Usart2);
    mClock.enable(ClockControl::Function::GpioA);
    mGpioA.configOutput(Gpio::Pin::Pin2, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Low);
    mGpioA.configInput(Gpio::Pin::Pin3);
    mGpioA.setAlternate(Gpio::Pin::Pin2, Gpio::AltFunc::Func7);
    mGpioA.setAlternate(Gpio::Pin::Pin3, Gpio::AltFunc::Func7);
    mDebug.config(115200);
    mFlash.set(Flash::Feature::InstructionCache, true);
    mFlash.set(Flash::Feature::DataCache, true);
    mClock.setSystemClock(168000000);
}

void StmSystem::handleInterrupt(uint32_t index)
{
    if (!System::tryHandleInterrupt(index)) mInt.handle(index);
}

void StmSystem::debugRead(char *msg, int len)
{
    System::Buffer buf(msg, len);
    mDebug.read(buf);
}

void StmSystem::debugWrite(const char *msg, int len)
{
    System::Buffer buf(msg, len);
    mDebug.write(buf);
}
