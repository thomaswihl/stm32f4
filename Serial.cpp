#include "Serial.h"

#include <cassert>
#include <cstdio>

Serial::Serial(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile USART*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mSpeed(0),
    mReadBuffer(READ_BUFFER_SIZE),
    mWriteBuffer(WRITE_BUFFER_SIZE)
{
    clockControl->addChangeHandler(this);
}

Serial::~Serial()
{
    mBase->SR.NF = 1;
    mBase->DR = 2;
    mBase->CR1.SBK = 1;

}

void Serial::setSpeed(uint32_t speed)
{
    uint32_t clock = mClockControl->clock(mClock);
    uint32_t accuracy = 8 * (2 - mBase->CR1.OVER8);
    uint32_t divider = clock / speed;
    mBase->BRR.DIV_MANTISSA = divider / accuracy;
    mBase->BRR.DIV_FRACTION = divider % accuracy;
    mSpeed = speed;
}

void Serial::setWordLength(Serial::WordLength dataBits)
{
    mBase->CR1.M = static_cast<uint32_t>(dataBits);
}

void Serial::setParity(Serial::Parity parity)
{
    mBase->CR1.PCE = (static_cast<uint32_t>(parity) & 2) >> 1;
    mBase->CR1.PS = static_cast<uint32_t>(parity) & 1;
}

void Serial::setStopBits(Serial::StopBits stopBits)
{
    mBase->CR2.STOP = static_cast<uint32_t>(stopBits);
}

void Serial::setHardwareFlowControl(Serial::HardwareFlowControl hardwareFlow)
{
    mBase->CR3.CTSE = hardwareFlow == HardwareFlowControl::Cts || hardwareFlow == HardwareFlowControl::CtsRts;
    mBase->CR3.RTSE = hardwareFlow == HardwareFlowControl::Rts || hardwareFlow == HardwareFlowControl::CtsRts;
}

void Serial::enable(bool enable)
{
    mBase->CR1.UE = enable;
    mBase->CR1.TE = enable;
    mBase->CR1.RE = enable;
}

void Serial::config(uint32_t speed, Serial::WordLength dataBits, Serial::Parity parity, Serial::StopBits stopBits, HardwareFlowControl hardwareFlow)
{
    enable(false);
    setSpeed(speed);
    setWordLength(dataBits);
    setParity(parity);
    setStopBits(stopBits);
    setHardwareFlowControl(hardwareFlow);
    enable(true);
}

void Serial::configDma(Dma::Stream *tx, Dma::Stream *rx)
{
    if (mDmaTx != 0)
    {
        mDmaTx->waitReady();
        delete mDmaTx;
    }
    if (mDmaRx != 0)
    {
        mDmaRx->waitReady();
        delete mDmaRx;
    }
    mDmaTx = tx;
    mDmaRx = rx;
}

void Serial::configInterrupt(InterruptController::Line* interrupt)
{
    if (mInterrupt != 0) delete mInterrupt;
    mBase->CR1.TCIE = 0;
    mBase->CR1.RXNEIE = 1;
    mInterrupt = interrupt;
    mInterrupt->setHandler(this);
    mInterrupt->enable();
}

int Serial::read(char* data, int size)
{
    while (mReadBuffer.used() == 0)
    {
    }

    return mReadBuffer.read(data, size);
}

int Serial::write(const char *data, int size)
{
    if (mDmaTx != 0)
    {

    }
    else if (mInterrupt != 0)
    {
        mWriteBuffer.write(data, size);
        if (!mBase->CR1.TCIE)
        {
            mBase->CR1.TCIE = 1;
            mBase->DR = *data++;
        }
    }
    else
    {
        for (int i = 0; i < size; ++i)
        {
            while (!mBase->SR.TC)
            {
            }
            mBase->DR = *data++;
        }
    }
    return size;
}

void Serial::handle(InterruptController::Index index)
{
    if (mBase->SR.RXNE)
    {
        char c = mBase->DR;
        if (c == '\r') std::putchar('\n');
        else std::putchar(c);
    }
    if (mBase->SR.TC)
    {
        char c;
        if (mWriteBuffer.pop(c)) mBase->DR = c;
        else mBase->CR1.TCIE = 0;
    }
}

void Serial::clockPrepareChange(uint32_t newClock)
{
}

void Serial::clockChanged(uint32_t newClock)
{
    if (mSpeed != 0) setSpeed(mSpeed);
}

