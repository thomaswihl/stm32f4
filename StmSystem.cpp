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

#include <cstdio>

StmSystem::StmSystem() :
    System(BaseAddress::SCB),
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
    mUsart1(*this, BaseAddress::USART1, System::Event::Component::USART1, &mRcc, ClockControl::Clock::APB2),
    mUsart2(*this, BaseAddress::USART2, System::Event::Component::USART2, &mRcc, ClockControl::Clock::APB1),
    mUsart3(*this, BaseAddress::USART3, System::Event::Component::USART3, &mRcc, ClockControl::Clock::APB1),
    mUart4(*this, BaseAddress::UART4, System::Event::Component::UART4, &mRcc, ClockControl::Clock::APB1),
    mUart5(*this, BaseAddress::UART5, System::Event::Component::UART5, &mRcc, ClockControl::Clock::APB1),
    mUsart6(*this, BaseAddress::USART6, System::Event::Component::USART6, &mRcc, ClockControl::Clock::APB2),
    mDebug(mUsart2),
    mFlash(BaseAddress::FLASH, mRcc),
    mFpu(BaseAddress::FPU)
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
}

void StmSystem::init()
{
    mRcc.enable(ClockControl::Function::Usart2);
    mRcc.enable(ClockControl::Function::GpioA);
    mRcc.enable(ClockControl::Function::Dma1);
    mGpioA.configOutput(Gpio::Pin::Pin2, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Low);
    mGpioA.configInput(Gpio::Pin::Pin3);
    mGpioA.setAlternate(Gpio::Pin::Pin2, Gpio::AltFunc::USART2);
    mGpioA.setAlternate(Gpio::Pin::Pin3, Gpio::AltFunc::USART2);
    mDebug.config(115200);
    mDebug.configDma(new Dma::Stream(mDma1, Dma::Stream::StreamIndex::Stream6, Dma::Stream::ChannelIndex::Channel4,
                                     new InterruptController::Line(mNvic, InterruptIndex::DMA1_Stream6)), nullptr);
    mDebug.configInterrupt(new InterruptController::Line(mNvic, InterruptIndex::USART2));
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

void StmSystem::printInfo()
{
    std::printf("\nSystem clock is %gMHz, AHB clock is %gMHz, APB1 is %gMHz, APB2 is %gMHz\n",
                mRcc.clock(ClockControl::Clock::System) / 1000000.0f,
                mRcc.clock(ClockControl::Clock::AHB) / 1000000.0f,
                mRcc.clock(ClockControl::Clock::APB1) / 1000000.0f,
                mRcc.clock(ClockControl::Clock::APB2) / 1000000.0f);
    std::printf("RAM  : %gk free, %gk used.\n", memFree() / 1024.0f, memUsed() / 1024.0f);
    std::printf("STACK: %gk free, %gk used.\n", stackFree() / 1024.0f, stackUsed() / 1024.0f);
}
