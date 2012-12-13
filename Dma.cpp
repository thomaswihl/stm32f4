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
    mBase->IFCR[index / 2] = (1 << flag) << (6 * (index % 2));
}

bool Dma::checkInterrupt(uint8_t stream, uint8_t flag)
{
    uint32_t index = static_cast<uint32_t>(stream);
    return (mBase->ISR[index / 2] & ((1 << flag) << (6 * (index % 2)))) != 0;
}

Dma::Stream::Stream(Dma &dma, Dma::Stream::StreamIndex stream, Dma::Stream::ChannelIndex channel) :
    mDma(dma),
    mStream(static_cast<uint8_t>(stream)),
    mChannel(static_cast<uint8_t>(channel))
{
    mDma.mBase->STREAM[mStream].CR.CHSEL = mChannel;

}

Dma::Stream::~Stream()
{
    waitReady();
    disable();
}

void Dma::Stream::enable()
{
    mDma.mBase->STREAM[mStream].CR.EN = 1;
}

void Dma::Stream::disable()
{
    mDma.mBase->STREAM[mStream].CR.EN = 0;
}

void Dma::Stream::waitReady()
{
    if (!mDma.mBase->STREAM[mStream].CR.EN) return;
    while (!mDma.checkInterrupt(mStream, Dma::TransferComplete) &&
           !mDma.checkInterrupt(mStream, Dma::TransferError) &&
           !mDma.checkInterrupt(mStream, Dma::DirectModeError) &&
           !mDma.checkInterrupt(mStream, Dma::FifoError))
    {

    }
}

