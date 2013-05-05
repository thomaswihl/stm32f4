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

Timer::Timer(System::BaseAddress base, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile TIMER*>(base)),
    mClock(clock)
{
    static_assert(sizeof(TIMER) == 0x54, "Struct has wrong size, compiler problem.");
    for (unsigned i = 0; i < LINE_COUNT; ++i) mLine[i] = nullptr;
    for (unsigned i = 0; i < EVENT_COUNT; ++i) mEvent[i] = nullptr;
}

void Timer::enable()
{
    mBase->DIER.UIE = 1;
    mBase->CR1.CEN = 1;
}

void Timer::disable()
{
    mBase->DIER.UIE = 0;
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

void Timer::setFrequency(const ClockControl &cc, uint32_t hz)
{
    uint32_t clock = cc.clock(mClock);
    uint32_t psc = clock / hz / 65535;
    setPrescaler(psc);
    setReload(clock / hz / (psc + 1));
}

void Timer::setOption(Option option)
{
    mBase->OR = static_cast<uint16_t>(option);
}

void Timer::setEvent(Timer::EventType type, System::Event *event)
{
    mEvent[static_cast<int>(type)] = event;
}

uint32_t Timer::capture(Timer::CaptureCompareIndex index)
{
    return mBase->CCR[static_cast<int>(index)];
}

void Timer::setCompare(Timer::CaptureCompareIndex index, uint32_t compare)
{
    mBase->CCR[static_cast<int>(index)] = compare;
}

void Timer::setCountMode(Timer::CountMode mode)
{
    switch (mode)
    {
    case Timer::CountMode::Up:
        mBase->CR1.CMS = 0;
        mBase->CR1.DIR = 0;
        break;
    case Timer::CountMode::Down:
        mBase->CR1.CMS = 0;
        mBase->CR1.DIR = 1;
        break;
    case Timer::CountMode::CenterAlignedDown:
        mBase->CR1.CMS = 1;
        break;
    case Timer::CountMode::CenterAlignedUp:
        mBase->CR1.CMS = 2;
        break;
    case Timer::CountMode::CenterAlignedUpDown:
        mBase->CR1.CMS = 3;
        break;

    }
}

void Timer::setInterrupt(Timer::InterruptType interruptType, InterruptController::Line* line)
{
    int index = static_cast<int>(interruptType);
    if (mLine[index] != nullptr) mLine[index]->disable();
    mLine[index] = line;
    mLine[index]->setCallback(this);
    mLine[index]->enable();
}

void Timer::setMaster(Timer::MasterMode mode)
{
    mBase->CR2.MMS = static_cast<uint32_t>(mode);
    mBase->SMCR.SMS = static_cast<uint32_t>(SlaveMode::Disabled);
}

void Timer::setSlave(Timer::SlaveMode mode, Trigger trigger, Prescaler inputPrescaler, Filter inputFilter, bool inputInvert)
{
    mBase->SMCR.SMS = static_cast<uint32_t>(mode);
    mBase->SMCR.TS = static_cast<uint32_t>(trigger);
    mBase->SMCR.ETF = static_cast<uint32_t>(inputFilter);
    mBase->SMCR.ETPS = static_cast<uint32_t>(inputPrescaler);
    mBase->SMCR.ETP = inputInvert ? 1 : 0;
    mBase->SMCR.TS = static_cast<uint32_t>(trigger);
}

void Timer::configCapture(Timer::CaptureCompareIndex index, Timer::Prescaler prescaler, Timer::Filter filter, Timer::CaptureEdge edge)
{
    CCMR_INPUT mr;
    mr.bits.CCS = 1;
    mr.bits.ICF = static_cast<uint16_t>(filter);
    mr.bits.ICPSC = static_cast<uint16_t>(prescaler);
    int i = static_cast<int>(index) & 2;
    int shift = (static_cast<int>(index) & 1) * 8;
    mBase->CCMR[i] = (mBase->CCMR[i] & ~(0xff << shift)) | (mr.value << shift);

    shift = static_cast<uint16_t>(index) * 4;
    uint16_t andmask = ~(static_cast<uint16_t>(CaptureEdge::Both) << shift);
    uint16_t ormask = static_cast<uint16_t>(edge) << shift;
    mBase->CCER = (mBase->CCER & andmask) | ormask;
}

void Timer::configCompare(Timer::CaptureCompareIndex index, Timer::CompareMode mode, CompareOutput output, CompareOutput complementaryOutput, bool latchCcr, bool fast, bool clearOnEtr)
{
    CCMR_OUTPUT mr;
    mr.bits.CCS = 0;     // configure as output
    mr.bits.OCFE = fast;
    mr.bits.OCPE = latchCcr;
    mr.bits.OCM = static_cast<uint16_t>(mode);
    mr.bits.OCCE = clearOnEtr;
    int i = static_cast<int>(index) & 2;
    int shift = (static_cast<int>(index) & 1) * 8;
    mBase->CCMR[i] = (mBase->CCMR[i] & ~(0xff << shift)) | (mr.value << shift);

    shift = static_cast<uint16_t>(index) * 4;
    uint16_t andmask = ~(static_cast<uint16_t>(0xf) << shift);
    uint16_t ormask = (static_cast<uint16_t>(output) << shift) | (static_cast<uint16_t>(complementaryOutput) << (shift + 2));

    mBase->CCER = (mBase->CCER & andmask) | ormask;
    mBase->BDTR.MOE = 1;
}

void Timer::enableCaptureCompareIrq(CaptureCompareIndex index, bool enable)
{
    switch (index)
    {
    case CaptureCompareIndex::Index1: mBase->DIER.CC1IE = enable; break;
    case CaptureCompareIndex::Index2: mBase->DIER.CC2IE = enable; break;
    case CaptureCompareIndex::Index3: mBase->DIER.CC3IE = enable; break;
    case CaptureCompareIndex::Index4: mBase->DIER.CC4IE = enable; break;
    }
}

void Timer::interruptCallback(InterruptController::Index index)
{
    __SR sr;
    sr.value = mBase->SR.value;

    if (sr.bits.UIF) postEvent(EventType::Update);
    if (sr.bits.CC1IF) postEvent(EventType::CaptureCompare1);
    if (sr.bits.CC2IF) postEvent(EventType::CaptureCompare2);
    if (sr.bits.CC3IF) postEvent(EventType::CaptureCompare3);
    if (sr.bits.CC4IF) postEvent(EventType::CaptureCompare4);
    mBase->SR.value = ~sr.value;
}

void Timer::postEvent(Timer::EventType type)
{
    int index = static_cast<int>(type);
    assert(index < EVENT_COUNT);
    if (mEvent[index] != nullptr) System::instance()->postEvent(mEvent[index]);
}
