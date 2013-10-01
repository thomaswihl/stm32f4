#include "ssd1306.h"

Ssd1306::Ssd1306(Spi &spi, Gpio::Pin &cs, Gpio::Pin& dataCommand, Gpio::Pin& reset) :
    mSpi(spi),
    mCs(cs),
    mDc(dataCommand),
    mReset(reset),
    mSpiEvent(*this)
{
    mCs.set();
    mFb = new uint8_t[FB_SIZE];
    std::memset(mFb, 0, FB_SIZE);
    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.mMaxSpeed = 10 * 1000 * 1000;
    mTransfer.mChipSelect = this;
    mTransfer.mClockPhase = Spi::ClockPhase::FirstTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::LowWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    mTransfer.mEvent = &mSpiEvent;
    mTransfer.mLength = 2;
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
    static const uint8_t COMMANDS[] =
    {
        Command::DisplayOff,
        Command::ClockDivide,
        0x80,
        Command::Multiplex,
        0x3F,
        Command::DisplayOffset,
        0x00,
        Command::StartLine0,
        Command::ChargePump,
        0x14,
        Command::MemoryAddressing,
        0x00,
        Command::SegRemap127,
        Command::ComScanDec,
        Command::ComPins,
        0x12,
        Command::Contrast,
        0xcf,
        Command::PreCharge,
        0xF1,
        Command::DeselectLevel,
        0x40,
        Command::EntireDisplayOnResume,
        Command::DisplayNormal,
        Command::DisplayOn,
    };
    sendCommands(COMMANDS, sizeof(COMMANDS));
}

void Ssd1306::setPixel(int x, int y, bool on)
{
    if (on) mFb[x + (y / 8) * DISPLAY_WIDTH] |= 1 << (y % 8);
    else mFb[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y % 8));
}

void Ssd1306::sendCommands(const uint8_t* cmds, unsigned size)
{
    mDc.reset();
    mTransfer.mWriteData = cmds;
    mTransfer.mLength = size;
    mSpi.transfer(&mTransfer);
}

void Ssd1306::select()
{
    mCs.reset();
}

void Ssd1306::deselect()
{
    mCs.set();
}

void Ssd1306::eventCallback(System::Event *event)
{
    if (!mDc.get())
    {
        mDc.set();
        mTransfer.mWriteData = mFb;
        mTransfer.mLength = FB_SIZE;
        mSpi.transfer(&mTransfer);
    }
}

void Ssd1306::sendData()
{
    static const uint8_t COMMANDS[] =
    {
        Command::LowColumn0,
        Command::HighColumn0,
        Command::StartLine0,
    };
    sendCommands(COMMANDS, sizeof(COMMANDS));
}
