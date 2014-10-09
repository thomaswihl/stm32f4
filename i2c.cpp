#include "i2c.h"

I2C::I2C(System::BaseAddress base, ClockControl *clockControl, ClockControl::Clock clock) :
    mBase(reinterpret_cast<volatile IIC*>(base)),
    mClockControl(clockControl),
    mClock(clock),
    mTransferBuffer(64)
{
    static_assert(sizeof(IIC) == 0x24, "Struct has wrong size, compiler problem.");
}



void I2C::enable(Device::Part part)
{
    mBase->CR1.PE = 1;
}

void I2C::disable(Device::Part part)
{
    mBase->CR1.PE = 0;
}

void I2C::setAddress(uint16_t address, I2C::AddressMode mode)
{
    if (mode == AddressMode::SevenBit)
    {
        mBase->OAR1.ADDMODE = 0;
        mBase->OAR1.ADD = (address & 0x7f) << 1;
    }
    else
    {
        mBase->OAR1.ADDMODE = 1;
        mBase->OAR1.ADD = address & 0x3ff;
    }
}

bool I2C::transfer(I2C::Transfer *transfer)
{
    bool success = mTransferBuffer.push(transfer);
    //printf("PUSH\n", ((transfer->mReadData != nullptr) ? "R" : "-"), transfer->mReadData, ((transfer->mWriteData != nullptr) ? "W" : "-"), transfer->mWriteData, transfer->mLength);
    //if (mBase->CR2.RXDMAEN == 0 && mBase->CR2.TXDMAEN == 0) nextTransfer();
    return success;
}

void I2C::configDma(Dma::Stream *write, Dma::Stream *read)
{
    Device::configDma(write, read);
    Dma::Stream::DataSize dataSize = Dma::Stream::DataSize::Byte;
    if (Device::mDmaWrite != nullptr)
    {
        mDmaWrite->config(Dma::Stream::Direction::MemoryToPeripheral, false, true, dataSize, dataSize, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaWrite->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::Quater);
    }
    if (Device::mDmaRead != nullptr)
    {
        mDmaRead->config(Dma::Stream::Direction::PeripheralToMemory, false, true, dataSize, dataSize, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
        mDmaRead->setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(&mBase->DR));
        mDmaWrite->configFifo(Dma::Stream::FifoThreshold::Quater);
    }
}

void I2C::configInterrupt(InterruptController::Line *event, InterruptController::Line *error)
{
    mEvent = event;
    if (mEvent != nullptr)
    {
        mEvent->setCallback(this);
        mEvent->enable();
    }
    mError = error;
    if (mError != nullptr)
    {
        mError->setCallback(this);
        mError->enable();
    }
}

void I2C::dmaReadComplete()
{
}

void I2C::dmaWriteComplete()
{
}

void I2C::clockCallback(ClockControl::Callback::Reason reason, uint32_t clock)
{
}

void I2C::interruptCallback(InterruptController::Index index)
{
    IIC::__SR1 sr1 = const_cast<const IIC::__SR1&>(mBase->SR1);
    printf("%02x\n", sr1);
    if (index == mEvent->index())
    {
        if (sr1.SB)
        {
            if (mActiveTransfer->mChip->addressMode() == AddressMode::SevenBit)
            {
                // EV6
                mBase->DR = (mActiveTransfer->mChip->address() << 1) | ((mActiveTransfer->mWriteData != 0 && mActiveTransfer->mWriteLength != 0) ? 0 : 1);
            }
            else
            {
                mBase->DR = 0xf0 | ((mActiveTransfer->mChip->address() >> 7) & 0x6);
            }
        }
        else if (sr1.ADDR)
        {
            // EV6
            IIC::__SR2 sr2 = const_cast<const IIC::__SR2&>(mBase->SR2);
            (void)sr2;
            if (mDmaWrite != nullptr && mActiveTransfer->mWriteData != nullptr)
            {
                mBase->CR2.DMAEN = 1;
                mDmaWrite->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(mActiveTransfer->mWriteData));
                mDmaWrite->setTransferCount(mActiveTransfer->mWriteLength);
                mDmaWrite->start();
            }
            else if (mDmaRead != nullptr && mActiveTransfer->mReadData != nullptr)
            {
                mBase->CR2.DMAEN = 1;
                mDmaRead->setAddress(Dma::Stream::End::Memory, reinterpret_cast<uint32_t>(mActiveTransfer->mReadData));
                mDmaRead->setTransferCount(mActiveTransfer->mReadLength);
                mDmaRead->start();
            }
        }
        else if (sr1.BTF)
        {
            // EV8
            mBase->CR1.STOP;
        }
        else if (sr1.ADD10)
        {
            mBase->DR = mActiveTransfer->mChip->address();
        }
        else if (sr1.STOPF)
        {

        }
        else if (sr1.RxNE)
        {

        }
        else if (sr1.TxE)
        {

        }
    }
    else
    {
        if (sr1.BERR)
        {
            printf("Bus error\n");
        }
        if (sr1.ARLO)
        {
            printf("Arbitration lost\n");
        }
        if (sr1.AF)
        {
            printf("Acknowledge failure\n");
        }
        if (sr1.OVR)
        {
            printf("Overrun/Underrun\n");
        }
        if (sr1.PECERR)
        {
            printf("PEC error\n");
        }
        if (sr1.TIMEOUT)
        {
            printf("Timeout\n");
        }
        if (sr1.SMBALERT)
        {
            printf("SMBus alert\n");
        }
    }
}

void I2C::setSpeed(uint32_t maxSpeed, Mode mode)
{
    uint32_t clock = mClockControl->clock(mClock);
    bool pe = mBase->CR1.PE;
    mBase->CR1.PE = 0;
    mBase->CR2.FREQ = (clock / 500000 + 1) / 2;
    switch (mode)
    {
    case Mode::Standard:
        mBase->CCR.CCR = std::max(4LU, (clock / maxSpeed + 1) / 2);
        mBase->TRISE.TRISE = clock / 1000000 + 1;
        break;
    case Mode::FastDuty2:
        mBase->CCR.CCR = std::max(1LU, (clock / maxSpeed + 2) / 3);
        mBase->TRISE.TRISE = clock / 3000000 + 1;
        break;
    case Mode::FastDuty16by9:
        mBase->CCR.CCR = std::max(1LU, (clock / maxSpeed + 24) / (9 + 16));
        mBase->TRISE.TRISE = clock / 3000000 + 1;
        break;

    }
    mBase->CR1.PE = pe;
}

void I2C::nextTransfer()
{
    Transfer* t;

    if (mTransferBuffer.back(t))
    {
        if ((t->mWriteLength == 0 || t->mWriteData == nullptr) && (t->mReadLength == 0 || t->mReadData == nullptr))
        {
            mTransferBuffer.pop(t);
            nextTransfer();
            return;
        }
        if (t->mChip == nullptr)
        {
            mTransferBuffer.pop(t);
            nextTransfer();
            return;
        }

        t->mChip->prepare();
        setSpeed(t->mChip->maxSpeed(), t->mChip->mode());
        mBase->CCR.FS = t->mChip->mode() != Mode::Standard;
        mBase->CCR.DUTY = t->mChip->mode() == Mode::FastDuty16by9;

        mActiveTransfer = t;
        if (mError != nullptr)
        {
            mBase->CR2.ITERREN = 1;
        }
        if (mEvent == nullptr)
        {
            // TODO: Implement polling
        }
        else
        {
            mBase->CR2.ITEVTEN = 1;
            mBase->CR1.START = 1;
        }
    }
    else
    {
        mBase->CR2.DMAEN = 0;
    }
}


















