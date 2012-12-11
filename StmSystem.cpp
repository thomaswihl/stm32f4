#include "StmSystem.h"

#include <cstdio>

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
    mRcc(static_cast<System::BaseAddress>(BaseAddress::RCC), 8000000),
    mExtI(static_cast<System::BaseAddress>(BaseAddress::EXTI), 23),
    mNvic(static_cast<System::BaseAddress>(BaseAddress::NVIC), 82),
    mDma1(static_cast<System::BaseAddress>(BaseAddress::DMA1)),
    mDma2(static_cast<System::BaseAddress>(BaseAddress::DMA2)),
    mUsart1(static_cast<System::BaseAddress>(BaseAddress::USART1), &mRcc, ClockControl::Clock::APB2),
    mUsart2(static_cast<System::BaseAddress>(BaseAddress::USART2), &mRcc, ClockControl::Clock::APB1),
    mUsart3(static_cast<System::BaseAddress>(BaseAddress::USART3), &mRcc, ClockControl::Clock::APB1),
    mUart4(static_cast<System::BaseAddress>(BaseAddress::UART4), &mRcc, ClockControl::Clock::APB1),
    mUart5(static_cast<System::BaseAddress>(BaseAddress::UART5), &mRcc, ClockControl::Clock::APB1),
    mUsart6(static_cast<System::BaseAddress>(BaseAddress::USART6), &mRcc, ClockControl::Clock::APB2),
    mDebug(mUsart2),
    mFlash(static_cast<System::BaseAddress>(BaseAddress::FLASH), mRcc)
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::init()
{
    mRcc.enable(ClockControl::Function::Usart2);
    mRcc.enable(ClockControl::Function::GpioA);
    mGpioA.configOutput(Gpio::Pin::Pin2, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Low);
    mGpioA.configInput(Gpio::Pin::Pin3);
    mGpioA.setAlternate(Gpio::Pin::Pin2, Gpio::AltFunc::USART2);
    mGpioA.setAlternate(Gpio::Pin::Pin3, Gpio::AltFunc::USART2);
    mDebug.config(115200);
    mDebug.configDma(new Dma::Stream(mDma1, Dma::Stream::StreamIndex::Stream6, Dma::Stream::ChannelIndex::Channel4), 0);
    mDebug.configInterrupt(new InterruptController::Line(mNvic, static_cast<InterruptController::Index>(InterruptIndex::USART2)));
    mFlash.set(Flash::Feature::InstructionCache, true);
    mFlash.set(Flash::Feature::DataCache, true);
    mRcc.setSystemClock(168000000);
}

int StmSystem::debugRead(char *msg, int len)
{
    return mDebug.read(msg, len);
}

int StmSystem::debugWrite(const char *msg, int len)
{
    return mDebug.write(msg, len);
}
