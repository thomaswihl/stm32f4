#include "StmSystem.h"

#include <cstdio>

StmSystem::StmSystem() :
    mGpioA(BaseAddress::GPIOA),
    mGpioB(BaseAddress::GPIOB),
    mGpioC(BaseAddress::GPIOC),
    mGpioD(BaseAddress::GPIOD),
    mGpioE(BaseAddress::GPIOE),
    mGpioF(BaseAddress::GPIOF),
    mGpioG(BaseAddress::GPIOG),
    mGpioH(BaseAddress::GPIOH),
    mGpioI(BaseAddress::GPIOI),
    mRcc(BaseAddress::RCC, 8000000),
    mExtI(BaseAddress::EXTI, 23),
    mNvic(BaseAddress::NVIC, 82),
    mSysTick(BaseAddress::STK, &mRcc, 1),
    mDma1(BaseAddress::DMA1),
    mDma2(BaseAddress::DMA2),
    mUsart1(*this, BaseAddress::USART1, &mRcc, ClockControl::Clock::APB2),
    mUsart2(*this, BaseAddress::USART2, &mRcc, ClockControl::Clock::APB1),
    mUsart3(*this, BaseAddress::USART3, &mRcc, ClockControl::Clock::APB1),
    mUart4(*this, BaseAddress::UART4, &mRcc, ClockControl::Clock::APB1),
    mUart5(*this, BaseAddress::UART5, &mRcc, ClockControl::Clock::APB1),
    mUsart6(*this, BaseAddress::USART6, &mRcc, ClockControl::Clock::APB2),
    mDebug(mUsart2),
    mFlash(BaseAddress::FLASH, mRcc),
    mFpu(BaseAddress::FPU)
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::handleTrap(uint32_t index)
{
    mDebug.configDma(nullptr, nullptr);
    mDebug.configInterrupt(nullptr);
    System::handleTrap(index);
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
    //mDebug.configDma(new Dma::Stream(mDma1, Dma::Stream::StreamIndex::Stream6, Dma::Stream::ChannelIndex::Channel4), 0);
    mDebug.configInterrupt(new InterruptController::Line(mNvic, static_cast<InterruptController::Index>(InterruptIndex::USART2)));
    mFlash.set(Flash::Feature::InstructionCache, true);
    mFlash.set(Flash::Feature::DataCache, true);
    mRcc.setSystemClock(168000000);
    mFpu.enable(FpuControl::AccessPrivileges::Full);
}

int StmSystem::debugRead(char *msg, int len)
{
    return mDebug.read(msg, len);
}

int StmSystem::debugWrite(const char *msg, int len)
{
    return mDebug.write(msg, len);
}
