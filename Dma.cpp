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

#include "Dma.h"

Dma::Dma(unsigned int base) :
    mBase(reinterpret_cast<volatile DMA*>(base))
{
    static_assert(sizeof(DMA) == 0xd0, "Struct has wrong size, compiler problem.");
}

void Dma::clearInterrupt(uint8_t stream, uint8_t flag)
{
    uint32_t index = static_cast<uint32_t>(stream);
    mBase->IFCR[index / 4] = flag << shiftFromIndex(index);
}

bool Dma::checkInterrupt(uint8_t stream, uint8_t flag)
{
    uint32_t index = static_cast<uint32_t>(stream);
    return (mBase->ISR[index / 4] & (flag << shiftFromIndex(index))) != 0;
}

uint8_t Dma::getInterruptStatus(uint8_t stream)
{
    uint32_t index = static_cast<uint32_t>(stream);
    return (mBase->ISR[index / 4] >> shiftFromIndex(index)) & 0x3f;
}

void Dma::clearInterruptStatus(uint8_t stream, uint8_t bits)
{
    uint32_t index = static_cast<uint32_t>(stream);
    mBase->IFCR[index / 4] = (bits & 0x3f) << shiftFromIndex(index);
}

unsigned int Dma::shiftFromIndex(uint32_t index)
{
    unsigned int shift = (index & 1) * 6;
    shift += ((index >> 1) & 1) * 16;
    return shift;
}

Dma::Stream::Stream(Dma &dma, Dma::Stream::StreamIndex stream, Dma::Stream::ChannelIndex channel, InterruptController::Line *interrupt) :
    mDma(dma),
    mStream(static_cast<uint8_t>(stream)),
    mChannel(static_cast<uint8_t>(channel)),
    mInterrupt(interrupt)
{
    mConfiguration.CR = 0;
    mConfiguration.CHSEL = mChannel;
    mConfiguration.TCIE = 1;
    mConfiguration.TEIE = 1;
    mConfiguration.DMEIE = 1;
    // This should always be on as HW sets it to zero if MemoryToMemory is active
    //mConfiguration.PFCTRL = 1;
    if (mInterrupt != nullptr)
    {
        mInterrupt->setCallback(this);
        mInterrupt->enable();
    }
}

Dma::Stream::~Stream()
{
    mDma.mBase->STREAM[mStream].CR.EN = 0;
}

void Dma::Stream::start()
{
    waitReady();
    mDma.mBase->STREAM[mStream].M0AR = mMemory;
    mDma.mBase->STREAM[mStream].PAR = mPeripheral;
    mDma.mBase->STREAM[mStream].NDTR = mCount;
    //mDma.mBase->STREAM[mStream].FCR.DMDIS = 1;
    mDma.mBase->STREAM[mStream].CR.CR = mConfiguration.CR;
    mDma.mBase->STREAM[mStream].CR.EN = 1;
}

void Dma::Stream::waitReady()
{
    while (mDma.mBase->STREAM[mStream].CR.EN)
    {

    }
}

void Dma::Stream::setBurstLength(Dma::Stream::End end, Dma::Stream::BurstLength burstLength)
{
    if (end == End::Memory) mConfiguration.MBURST = static_cast<uint32_t>(burstLength);
    else mConfiguration.PBURST = static_cast<uint32_t>(burstLength);
}

void Dma::Stream::setPriority(Dma::Stream::Priority priority)
{
    mConfiguration.PL = static_cast<uint32_t>(priority);
}

void Dma::Stream::setDataSize(Dma::Stream::End end, Dma::Stream::DataSize dataSize)
{
    if (end == End::Memory) mConfiguration.MSIZE = static_cast<uint32_t>(dataSize);
    else mConfiguration.PSIZE = static_cast<uint32_t>(dataSize);
}

void Dma::Stream::setIncrement(Dma::Stream::End end, bool increment)
{
    if (end == End::Memory) mConfiguration.MINC = increment ? 1 : 0;
    else mConfiguration.PINC = increment ? 1 : 0;
}

void Dma::Stream::setDirection(Dma::Stream::Direction direction)
{
     mConfiguration.DIR = static_cast<uint32_t>(direction);
}

void Dma::Stream::setAddress(Dma::Stream::End end, System::BaseAddress address)
{
    if (end == End::Memory) mMemory = address;
    else mPeripheral = address;
}

void Dma::Stream::setCallback(Dma::Stream::Callback *callback)
{
    mCallback = callback;
}

void Dma::Stream::configure(Dma::Stream::Direction direction, bool peripheralIncrement, bool memoryIncrement, Dma::Stream::DataSize peripheralDataSize, Dma::Stream::DataSize memoryDataSize, Dma::Stream::BurstLength peripheralBurst, Dma::Stream::BurstLength memoryBurst)
{
    setBurstLength(End::Memory, memoryBurst);
    setBurstLength(End::Peripheral, peripheralBurst);

    setDataSize(End::Memory, memoryDataSize);
    setDataSize(End::Peripheral, peripheralDataSize);

    setIncrement(End::Memory, memoryIncrement);
    setIncrement(End::Peripheral, peripheralIncrement);

    setDirection(direction);
}

void Dma::Stream::interruptCallback(InterruptController::Index index)
{
    uint8_t status = mDma.getInterruptStatus(mStream);
    mDma.clearInterruptStatus(mStream, status);
    if (mCallback != nullptr)
    {
        mCallback->dmaCallback(static_cast<InterruptFlag>(status));
    }
}


void Dma::Stream::setTransferCount(uint16_t count)
{
    mCount = count;
}

