#include "Dma.h"

Dma::Dma(unsigned int base) :
    mBase(reinterpret_cast<volatile DMA*>(base))
{
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

