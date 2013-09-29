#include "ssd1306.h"

Ssd1306::Ssd1306(Spi &spi, Gpio::Pin &cs, Gpio::Pin& dataCommand, Gpio::Pin& reset) :
    mSpi(spi),
    mCs(cs),
    mDc(dataCommand),
    mReset(reset)
{
    mCs.set();
    std::memset(mFb, 0, sizeof(mFb));
    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.maxSpeed = 10000000;
    mTransfer.mChipSelect = 0;
    mTransfer.mClockPhase = Spi::ClockPhase::FirstTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::LowWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    mTransfer.mEvent = 0;
    mTransfer.mReadData = new uint8_t[2];
    mTransfer.mReadDataCount = 2;
    mTransfer.mWriteData = new uint8_t[2];
    mTransfer.mWriteDataCount = 2;
}

void Ssd1306::reset()
{
    mReset.set();
    System::instance()->usleep(100000);
    mReset.reset();
    System::instance()->usleep(100000);
    mReset.set();
}

void Ssd1306::init()
{
    sendCommand(Command::DisplayOff);
    sendCommand(Command::ClockDivide);
    sendCommand(0x80);
    sendCommand(Command::Multiplex);
    sendCommand(0x3F);
    sendCommand(Command::DisplayOffset);
    sendCommand(0x00);
    sendCommand(Command::StartLine0);
    sendCommand(Command::ChargePump);
    sendCommand(0x14);
    sendCommand(Command::MemoryAddressing);
    sendCommand(0x00);
    sendCommand(Command::SegRemap127);
    sendCommand(Command::ComScanDec);
    sendCommand(Command::ComPins);
    sendCommand(0x12);
    sendCommand(Command::Contrast);
    sendCommand(0xcf);
    sendCommand(Command::PreCharge);
    sendCommand(0xF1);
    sendCommand(Command::DeselectLevel);
    sendCommand(0x40);
    sendCommand(Command::EntireDisplayOnResume);
    sendCommand(Command::DisplayNormal);
    sendCommand(Command::DisplayOn);
    sendData();
}

void Ssd1306::setPixel(int x, int y, bool on)
{
    if (on) mFb[x + (y / 8) * DISPLAY_WIDTH] |= 1 << (y % 8);
    else mFb[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y % 8));
}

void Ssd1306::sendCommand(Ssd1306::Command cmd)
{
    sendCommand(static_cast<uint8_t>(cmd));
}

void Ssd1306::sendCommand(uint8_t cmd)
{
    char buf = cmd;

    mDc.reset();
    mCs.reset();
    //mSpi.write(&buf, 1);
    mCs.set();
}

void Ssd1306::sendData()
{
    sendCommand(Command::LowColumn0);
    sendCommand(Command::HighColumn0);
    sendCommand(Command::StartLine0);
    mDc.set();
    mCs.reset();
    //mSpi.write(mFb, sizeof(mFb));
    mCs.set();
}
