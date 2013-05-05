#include "tlc5940.h"

Tlc5940::Tlc5940(Spi<char> &spi, Gpio::Pin &xlat, Gpio::Pin &blank, Timer &gsclkPwm, Timer &gsclkLatch) :
    mSpi(spi),
    mXlat(xlat),
    mBlank(blank),
    mPwm(gsclkPwm),
    mLatch(gsclkLatch),
    mLatchEvent(*this),
    mSpiEvent(*this),
    mNewData(false)
{
    mGrayScaleData = new char[GRAYSCALE_DATA_COUNT];
    std::memset(mGrayScaleData, 0, GRAYSCALE_DATA_COUNT);
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::LowWhenIdle, Spi<char>::ClockPhase::FirstTransition, Spi<char>::Endianess::MsbFirst);
    printf("BR = %lu\n", mSpi.setSpeed(1 * 100000));
    mSpi.enable(Device::All);
    mBlank.set();
    mXlat.reset();
    gsclkPwm.setMaster(Timer::MasterMode::Update);
    gsclkPwm.setCountMode(Timer::CountMode::Up);
    gsclkPwm.setCompare(Timer::CaptureCompareIndex::Index1, gsclkPwm.reload() / 2);
    gsclkPwm.configCompare(Timer::CaptureCompareIndex::Index1, Timer::CompareMode::PwmActiveWhenHigher, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    gsclkLatch.setSlave(Timer::SlaveMode::ExternalClock, Timer::Trigger::Internal2);
    gsclkLatch.setReload(4096);
    gsclkLatch.setEvent(Timer::EventType::Update, &mLatchEvent);
    gsclkPwm.enable();
    gsclkLatch.enable();
}

// 00 01 11 22 23 33 44 45 55
// TIMER1 0x40010000
// TIMER2 0x40000000

void Tlc5940::setOutput(int index, uint16_t value)
{
    if (index < 0 || index > 15) return;

    int i = 15 - index;

    if ((i % 2) == 0)
    {
        mGrayScaleData[i + i / 2] = value >> 4;
        mGrayScaleData[i + i / 2 + 1] = (mGrayScaleData[i + i / 2 + 1] & 0x0f) | ((value << 4) & 0xf0);
    }
    else
    {
        mGrayScaleData[i + i / 2] = (mGrayScaleData[i + i / 2 + 1] & 0xf0) | ((value >> 8) & 0x0f);
        mGrayScaleData[i + i / 2 + 1] = value;
    }
}

void Tlc5940::send()
{
    mSpi.write(mGrayScaleData, GRAYSCALE_DATA_COUNT, &mSpiEvent);
}

void Tlc5940::eventCallback(System::Event *event)
{
    if (event == &mSpiEvent)
    {
        mNewData = true;
    }
    else if (event == &mLatchEvent)
    {
        mBlank.set();
        System::instance()->nspin(30);
        if (mNewData)
        {
            mNewData = false;
            mXlat.set();
            System::instance()->nspin(30);
            mXlat.reset();
            System::instance()->nspin(30);
        }
        mBlank.reset();
    }
}
