#include "ssd1306.h"

Ssd1306::Ssd1306(Spi<char> &spi, Gpio::Pin &cs, Gpio::Pin& dataCommand, Gpio::Pin& reset) :
    mSpi(spi),
    mCs(cs),
    mDc(dataCommand),
    mReset(reset)
{
    mCs.set();
    for (unsigned i = 0; i < sizeof(mFb) / sizeof(mFb[0]); ++i) mFb[i] = i;
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::LowWhenIdle, Spi<char>::ClockPhase::FirstTransition, Spi<char>::Endianess::MsbFirst);
    mSpi.setSpeed(1 * 1000* 1000);
    mSpi.enable(Device::All);
}

void Ssd1306::init()
{
    mReset.reset();
    System::instance()->usleep(10000);
    mReset.set();
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

void Ssd1306::sendCommand(Ssd1306::Command cmd)
{
    sendCommand(static_cast<uint8_t>(cmd));
}

void Ssd1306::sendCommand(uint8_t cmd)
{
    char buf = cmd;

    mDc.reset();
    mCs.reset();
    mSpi.write(&buf, 1);
    mCs.set();
}

void Ssd1306::sendData()
{
    sendCommand(Command::LowColumn0);
    sendCommand(Command::HighColumn0);
    sendCommand(Command::StartLine0);
    mDc.set();
    mCs.reset();
    mSpi.write(mFb, sizeof(mFb));
    mCs.set();
}
