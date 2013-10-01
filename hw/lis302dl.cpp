#include "lis302dl.h"

LIS302DL::LIS302DL(Spi &spi) :
    mTransferCompleteEvent(*this),
    mSpi(spi),
    mBuffer(new char[2]),
    mLine1(nullptr),
    mLine2(nullptr),
    mDataReadyEvent(nullptr)
{
    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.mMaxSpeed = 10000000;
    mTransfer.mChipSelect = 0;
    mTransfer.mClockPhase = Spi::ClockPhase::SecondTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::HighWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    mTransfer.mEvent = nullptr;
    mReadBuffer = new uint8_t[2];
    mWriteBuffer = new uint8_t[2];
    mTransfer.mReadData = mReadBuffer;
    mTransfer.mWriteData = mWriteBuffer;
    mTransfer.mLength = 2;
}

void LIS302DL::enable()
{
    set(Register::Control1, DataRate100 | PowerUp | Range2G | EnableX | EnableY | EnableZ);
    set(Register::Control2, Spi4Wire | DisableFilter);
    set(Register::Control3, InterruptActiveHigh | InterruptPushPull | (static_cast<uint8_t>(InterruptConfig::DataReady) << Interrupt1ConfigShift) | (static_cast<uint8_t>(InterruptConfig::Click) << Interrupt2ConfigShift));
    mLine1->enable(ExternalInterrupt::Trigger::Rising);
    mLine2->enable(ExternalInterrupt::Trigger::Rising);
}

void LIS302DL::disable()
{
    mSpi.disable(Device::All);
    set(Register::Control1, PowerDown);
}

void LIS302DL::configInterrupt(ExternalInterrupt::Line *line1, ExternalInterrupt::Line *line2)
{
    mLine1 = line1;
    if (mLine1 != nullptr) mLine1->setCallback(this);
    mLine2 = line2;
    if (mLine2 != nullptr) mLine2->setCallback(this);
}

void LIS302DL::setDataReadyEvent(System::Event *event)
{
    mDataReadyEvent = event;
}

int8_t LIS302DL::x()
{
    return static_cast<int8_t>(get(Register::OutX));
}


int8_t LIS302DL::y()
{
    return static_cast<int8_t>(get(Register::OutY));
}

int8_t LIS302DL::z()
{
    return static_cast<int8_t>(get(Register::OutZ));
}

void LIS302DL::eventCallback(System::Event *event)
{
}

void LIS302DL::interruptCallback(InterruptController::Index index)
{
    if (index == mLine1->index())
    {
        if (mDataReadyEvent != 0) System::instance()->postEvent(mDataReadyEvent);
    }
    else if (index == mLine2->index())
    {
        printf("LINE2 IRQ\n");
    }
}

void LIS302DL::set(LIS302DL::Register reg, uint8_t value)
{
    mWriteBuffer[0] = WRITE | ADDR_CONST | static_cast<uint8_t>(reg);
    mWriteBuffer[1] = value;
    mSpi.transfer(&mTransfer);
}

uint8_t LIS302DL::get(LIS302DL::Register reg)
{
    mWriteBuffer[0] = READ | ADDR_CONST | static_cast<uint8_t>(reg);
    mWriteBuffer[1] = 0;
    mSpi.transfer(&mTransfer);
    // TODO: Wait for transfer to finish
    return 0;//buf[1];
}


