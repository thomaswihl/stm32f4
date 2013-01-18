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

#ifndef DEVICE_H
#define DEVICE_H

#include "System.h"
#include "Dma.h"

class Device : public Dma::Stream::Callback, public InterruptController::Callback
{
public:
    Device();
    enum Part { Read = 1, Write = 2, All = 3 };

    virtual void enable(Part part) = 0;
    virtual void disable(Part part) = 0;
    virtual void dmaReadComplete() = 0;
    virtual void dmaWriteComplete() = 0;

    virtual void configDma(Dma::Stream* write, Dma::Stream* read);
    virtual void configInterrupt(InterruptController::Line* interrupt);

protected:
    InterruptController::Line* mInterrupt;
    Dma::Stream* mDmaWrite;
    Dma::Stream* mDmaRead;

    virtual void dmaCallback(Dma::Stream* stream, Dma::Stream::Callback::Reason reason);


};

#endif // DEVICE_H
