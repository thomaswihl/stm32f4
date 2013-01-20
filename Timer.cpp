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

#include "Timer.h"

#include <cassert>

Timer::Timer(System::BaseAddress base, InterruptController::Line &line) :
    mBase(reinterpret_cast<volatile TIMER*>(base)),
    mLine(line)
{
    static_assert(sizeof(TIMER) == 0x54, "Struct has wrong size, compiler problem.");
    for (unsigned i = 0; i < EVENT_COUNT; ++i) mEvent[i] = nullptr;
    mLine.setCallback(this);
}

void Timer::enable()
{
    mBase->CR1.CEN = 1;
    mLine.enable();
}

void Timer::disable()
{
    mLine.disable();
    mBase->CR1.CEN = 0;
}

void Timer::dmaReadComplete()
{
}

void Timer::dmaWriteComplete()
{
}

void Timer::setCounter(uint32_t counter)
{
    mBase->CNT = counter;
}

uint32_t Timer::counter()
{
    return mBase->CNT;
}

void Timer::setPrescaler(uint16_t prescaler)
{
    mBase->PSC = prescaler;
}

void Timer::setReload(uint32_t reload)
{
    mBase->ARR = reload;
}

void Timer::setOption(Option option)
{
    mBase->OR = static_cast<uint16_t>(option);
}

void Timer::setEvent(Timer::EventType type, System::Event *event)
{
    mEvent[static_cast<int>(type)] = event;
}

uint32_t Timer::captureCompare(Timer::CaptureCompareIndex index)
{
    return mBase->CCR[static_cast<int>(index)];
}

void Timer::configCapture(Timer::CaptureCompareIndex index, Timer::CapturePrescaler prescaler, Timer::CaptureFilter filter, Timer::CaptureEdge edge)
{
    CCMR_INPUT mr;
    mr.CCS = 1;
    mr.ICF = static_cast<uint16_t>(filter);
    mr.ICPSC = static_cast<uint16_t>(prescaler);
    int i = static_cast<int>(index) & 2;
    int shift = (static_cast<int>(index) & 1) * 8;
    mBase->CCMR[i] = (mBase->CCMR[i] & ~(0xff << shift)) | (mr.CCMR_INPUT << shift);

    shift = static_cast<uint16_t>(index) * 4;
    uint16_t andmask = ~(static_cast<uint16_t>(CaptureEdge::Both) << shift);
    uint16_t ormask = static_cast<uint16_t>(edge) << shift;
    mBase->CCER = (mBase->CCER & andmask) | ormask;
}

void Timer::enableCaptureCompare(CaptureCompareIndex index, CaptureCompareEnable enable)
{
    int shift = static_cast<uint16_t>(index) * 4;
    uint16_t andmask = ~(static_cast<uint16_t>(CaptureCompareEnable::All) << shift);
    uint16_t ormask = static_cast<uint16_t>(enable) << shift;
    mBase->CCER = (mBase->CCER & andmask) | ormask;
}

void Timer::interruptCallback(InterruptController::Index index)
{
    if (mBase->SR.UIF) postEvent(EventType::Update);
    if (mBase->SR.CC1IF) postEvent(EventType::CaptureCompare1);
}

void Timer::postEvent(Timer::EventType type)
{
    int index = static_cast<int>(type);
    assert(index < EVENT_COUNT);
    if (mEvent[index] != nullptr) System::instance()->postEvent(mEvent[index]);
}
