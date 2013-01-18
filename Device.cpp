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

#include "Device.h"

Device::Device() :
    mDmaWrite(nullptr),
    mDmaRead(nullptr)
{

}

void Device::configDma(Dma::Stream *write, Dma::Stream *read)
{
    mDmaWrite = write;
    mDmaRead = read;
    if (mDmaWrite != nullptr)
    {
        mDmaWrite->setCallback(this);
    }
    if (mDmaRead != nullptr)
    {
        mDmaRead->setCallback(this);
    }
}

void Device::configInterrupt(InterruptController::Line* interrupt)
{
    mInterrupt = interrupt;
    if (mInterrupt != nullptr)
    {
        mInterrupt->setCallback(this);
        mInterrupt->enable();
    }
}

void Device::dmaCallback(Dma::Stream* stream, Dma::Stream::Callback::Reason reason)
{
    if (stream == mDmaWrite)
    {
        if (reason != Dma::Stream::Callback::Reason::TransferComplete)
        {
            // Ooops something went wrong (probably our configuration, anyway, disable DMA.
            configDma(nullptr, mDmaRead);
            System::instance()->printError("Device", "DMA write transfer failed");
        }
        dmaWriteComplete();
    }
    else if (stream == mDmaRead)
    {
        if (reason != Dma::Stream::Callback::Reason::TransferComplete)
        {
            // Ooops something went wrong (probably our configuration, anyway, disable DMA.
            configDma(mDmaWrite, nullptr);
            System::instance()->printError("Device", "DMA read transfer failed");
        }
        dmaReadComplete();
    }
}

