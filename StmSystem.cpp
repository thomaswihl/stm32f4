/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StmSystem.h"
#include "version.h"

#include <cstdio>

StmSystem::StmSystem() :
    mSysTick(BaseAddress::STK, &mRcc, 100),
    System(BaseAddress::SCB, mSysTick),
    mGpioA(BaseAddress::GPIOA),
    mGpioB(BaseAddress::GPIOB),
    mGpioC(BaseAddress::GPIOC),
    mGpioD(BaseAddress::GPIOD),
    mGpioE(BaseAddress::GPIOE),
    mGpioF(BaseAddress::GPIOF),
    mGpioG(BaseAddress::GPIOG),
    mGpioH(BaseAddress::GPIOH),
    mGpioI(BaseAddress::GPIOI),
    mTim1(BaseAddress::TIM1),
    mTim2(BaseAddress::TIM2),
    mTim3(BaseAddress::TIM3),
    mTim4(BaseAddress::TIM4),
    mTim5(BaseAddress::TIM5),
    mTim6(BaseAddress::TIM6),
    mTim7(BaseAddress::TIM7),
    mTim8(BaseAddress::TIM8),
    mTim9(BaseAddress::TIM9),
    mTim10(BaseAddress::TIM10),
    mTim11(BaseAddress::TIM11),
    mTim12(BaseAddress::TIM12),
    mTim13(BaseAddress::TIM13),
    mTim14(BaseAddress::TIM14),
    mRcc(BaseAddress::RCC, 8000000),
    mExtI(BaseAddress::EXTI, 23),
    mNvic(BaseAddress::NVIC, 82),
    mSysCfg(BaseAddress::SYSCFG),
    mDma1(BaseAddress::DMA1),
    mDma2(BaseAddress::DMA2),
//    mUsart1(*this, BaseAddress::USART1, &mRcc, ClockControl::Clock::APB2),
    mUsart2(*this, BaseAddress::USART2, &mRcc, ClockControl::Clock::APB1),
//    mUsart3(*this, BaseAddress::USART3, &mRcc, ClockControl::Clock::APB1),
//    mUart4(*this, BaseAddress::UART4, &mRcc, ClockControl::Clock::APB1),
//    mUart5(*this, BaseAddress::UART5, &mRcc, ClockControl::Clock::APB1),
//    mUsart6(*this, BaseAddress::USART6, &mRcc, ClockControl::Clock::APB2),
    mDebug(mUsart2),
    mSpi1(*this, BaseAddress::SPI1, &mRcc, ClockControl::Clock::APB2),
//    mSpi2(*this, BaseAddress::SPI2, &mRcc, ClockControl::Clock::APB1),
//    mSpi3(*this, BaseAddress::SPI3, &mRcc, ClockControl::Clock::APB1),
    mFlash(BaseAddress::FLASH, mRcc, Flash::AccessSize::x32),
    mFpu(BaseAddress::FPU),
    mIWdg(BaseAddress::IWDG)
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::handleTrap(System::TrapIndex index, unsigned int* stackPointer)
{
    mDebug.configDma(nullptr, nullptr);
    mDebug.configInterrupt(nullptr);
    System::handleTrap(index, stackPointer);
    // wait for last byte to be written
    for (int i = 0; i < 10000; ++i);
    mRcc.resetClock();
}

void StmSystem::init()
{
    mRcc.setSystemClock(168000000);
    mRcc.enable(ClockControl::Function::Usart2);
    mRcc.enable(ClockControl::Function::GpioA);
    mRcc.enable(ClockControl::Function::Dma1);

//    mDebug.config(9600);
    mDebug.config(921600);
    mDebug.configDma(new Dma::Stream(mDma1, Dma::Stream::StreamIndex::Stream6, Dma::Stream::ChannelIndex::Channel4,
                                     new InterruptController::Line(mNvic, InterruptIndex::DMA1_Stream6)),
                     nullptr
//                     new Dma::Stream(mDma1, Dma::Stream::StreamIndex::Stream5, Dma::Stream::ChannelIndex::Channel4,
//                                                          new InterruptController::Line(mNvic, InterruptIndex::DMA1_Stream5))
                     );
    mDebug.configInterrupt(new InterruptController::Line(mNvic, InterruptIndex::USART2));
    mDebug.readFifo(256);
    mDebug.writeFifo(256);
    mDebug.enable(Device::All);

    // USART2 TX
    mGpioA.configOutput(Gpio::Index::Pin2, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Low);
    mGpioA.setAlternate(Gpio::Index::Pin2, Gpio::AltFunc::USART2);
    // USART2 RX
    mGpioA.configInput(Gpio::Index::Pin3);
    mGpioA.setAlternate(Gpio::Index::Pin3, Gpio::AltFunc::USART2);

    mFlash.set(Flash::Feature::InstructionCache, true);
    mFlash.set(Flash::Feature::DataCache, true);
    mFlash.set(Flash::Feature::Prefetch, true);
    mFpu.enable(FpuControl::AccessPrivileges::Full);
}

void StmSystem::debugRead(char *msg, unsigned int len)
{
    mDebug.read(msg, len);
}

void StmSystem::debugWrite(const char *msg, unsigned int len)
{
    mDebug.write(msg, len);
}

void StmSystem::printInfo()
{
    updateBogoMips();
    std::printf("CLOCK   : System = %luMHz, AHB = %luMHz, APB1 = %luMHz, APB2 = %luMHz\n",
                mRcc.clock(ClockControl::Clock::System) / 1000000,
                mRcc.clock(ClockControl::Clock::AHB) / 1000000,
                mRcc.clock(ClockControl::Clock::APB1) / 1000000,
                mRcc.clock(ClockControl::Clock::APB2) / 1000000);
    std::printf("BOGOMIPS: %lu.%lu\n", bogoMips() / 1000000, bogoMips() % 1000000);
    std::printf("RAM     : %luk heap free, %luk heap used, %luk bss used, %lik data used.\n", (memFree() + 512) / 1024, (memUsed() + 512) / 1024, (memBssUsed() + 512) / 1024, (memDataUsed() + 512) / 1024);
    std::printf("STACK   : %luk free, %luk used, %luk max used.\n", (stackFree() + 512) / 1024, (stackUsed() + 512) / 1024, (stackMaxUsed() + 512) / 1024);
    std::printf("BUILD   : %s\n", GIT_VERSION);
    std::printf("DATE    : %s\n", BUILD_DATE);
}
