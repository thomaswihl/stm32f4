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
    mInterrupt(interrupt),
    mCallback(nullptr),
    mPeripheral(0),
    mMemory0(0),
    mMemory1(0),
    mCount(0)
{
    mStreamConfig.CR = 0;
    mStreamConfig.CHSEL = mChannel;
    mStreamConfig.TCIE = 1;
    mStreamConfig.TEIE = 1;
    mStreamConfig.DMEIE = 1;
    mFifoConfig.FCR = 0;
    mFifoConfig.FTH = static_cast<uint32_t>(FifoThreshold::Half);

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
    mDma.mBase->STREAM[mStream].M0AR = mMemory0;
    mDma.mBase->STREAM[mStream].M1AR = mMemory1;
    mDma.mBase->STREAM[mStream].PAR = mPeripheral;
    mDma.mBase->STREAM[mStream].NDTR = mCount;
    mDma.mBase->STREAM[mStream].FCR.FCR = mFifoConfig.FCR;
    mDma.mBase->STREAM[mStream].CR.CR = mStreamConfig.CR;
    mDma.mBase->STREAM[mStream].CR.EN = 1;
}

void Dma::Stream::waitReady()
{
    if (mDma.mBase->STREAM[mStream].CR.EN)
    {
        System::instance()->usleep(100000);
        mDma.mBase->STREAM[mStream].CR.EN = 0;
    }
}

void Dma::Stream::setBurstLength(Dma::Stream::End end, Dma::Stream::BurstLength burstLength)
{
    if (end == End::Memory) mStreamConfig.MBURST = static_cast<uint32_t>(burstLength);
    else mStreamConfig.PBURST = static_cast<uint32_t>(burstLength);
}

void Dma::Stream::setPriority(Dma::Stream::Priority priority)
{
    mStreamConfig.PL = static_cast<uint32_t>(priority);
}

void Dma::Stream::setDataSize(Dma::Stream::End end, Dma::Stream::DataSize dataSize)
{
    if (end == End::Memory) mStreamConfig.MSIZE = static_cast<uint32_t>(dataSize);
    else mStreamConfig.PSIZE = static_cast<uint32_t>(dataSize);
}

void Dma::Stream::setIncrement(Dma::Stream::End end, bool increment)
{
    if (end == End::Memory) mStreamConfig.MINC = increment ? 1 : 0;
    else mStreamConfig.PINC = increment ? 1 : 0;
}

void Dma::Stream::setDirection(Dma::Stream::Direction direction)
{
     mStreamConfig.DIR = static_cast<uint32_t>(direction);
}

void Dma::Stream::setAddress(Dma::Stream::End end, System::BaseAddress address)
{
    if (end == End::Memory0) mMemory0 = address;
    else if (end == End::Memory1) mMemory1 = address;
    else mPeripheral = address;
}

void Dma::Stream::setCallback(Callback* callback)
{
    mCallback = callback;
}

void Dma::Stream::config(Dma::Stream::Direction direction, bool peripheralIncrement, bool memoryIncrement, Dma::Stream::DataSize peripheralDataSize, Dma::Stream::DataSize memoryDataSize, Dma::Stream::BurstLength peripheralBurst, Dma::Stream::BurstLength memoryBurst)
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
    // get and clear interrupt flags
    uint8_t status = mDma.getInterruptStatus(mStream);
    mDma.clearInterruptStatus(mStream, status);
    if (mCallback != nullptr)
    {
        Callback::Reason reason = Callback::Reason::TransferComplete;
        // someone cares about our result so lets find it
        bool fifoError = (status & FifoError) != 0;
        bool directModeError = (status & DirectModeError) != 0;
        bool transferError = (status & TransferError) != 0;

        // This can only happen in peripheral to memory transfer with no memory increase.
        // It means that 1 transfer didn't happen and there will be 2 successive transfers
        if (directModeError) reason = Callback::Reason::DirectModeError;

        // In direct transfer FIFO error signals an over/underrun and isn't serios, so we can ignore it.
        // In FIFO mode this is fatal (as no data has been transmitted) and caused by wrong configuration of FIFO.
        if (fifoError && !mDma.mBase->STREAM[mStream].FCR.DMDIS) reason = Callback::Reason::FifoError;

        // A bus errror triggers this as well as a write to memory register during a transfer, pretty fatal.
        if (transferError) reason = Callback::Reason::DirectModeError;
        mCallback->dmaCallback(this, reason);
    }
}

bool Dma::Stream::complete()
{
    return mDma.mBase->STREAM[mStream].CR.EN == 0;
}


void Dma::Stream::setTransferCount(uint16_t count)
{
    mCount = count;
}

uint16_t Dma::Stream::transferCount()
{
    return mCount;
}

void Dma::Stream::setFlowControl(Dma::Stream::FlowControl flowControl)
{
    mStreamConfig.PFCTRL = (flowControl == FlowControl::Dma) ? 0 : 1;
}

void Dma::Stream::setCircular(bool circular)
{
    mDma.mBase->STREAM[mStream].CR.CIRC = circular ? 1 : 0;
}


void Dma::Stream::configFifo(Dma::Stream::FifoThreshold threshold)
{
    if (threshold == FifoThreshold::Disable)
    {
        mFifoConfig.DMDIS = 0;
    }
    else
    {
        mFifoConfig.DMDIS = 1;
        mFifoConfig.FTH = static_cast<uint32_t>(threshold);
    }
}
