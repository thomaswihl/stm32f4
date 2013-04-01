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
    mSpi.setSpeed(10 * 1000* 1000);
    mSpi.enable(Device::All);
    mBlank.set();
    mXlat.reset();
    gsclkPwm.setMaster(Timer::MasterMode::Update);
    gsclkPwm.setCountMode(Timer::CountMode::Up);
    gsclkPwm.setCompare(Timer::CaptureCompareIndex::Index2, gsclkPwm.reload() / 2);
    gsclkPwm.configCompare(Timer::CaptureCompareIndex::Index2, Timer::CompareMode::PwmActiveWhenHigher, Timer::CompareOutput::Disabled, Timer::CompareOutput::ActiveHigh);
    gsclkLatch.setSlave(Timer::SlaveMode::ExternalClock, Timer::Trigger::Internal0);
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
    static int count = 0;
    static int inc = 1;
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
            count += inc;
            if (count > 170 || count < 0) inc = -inc;
            for (int i = 1; i <= 16; ++i) setOutput(i - 1, std::max(0, 4095 - 4096 * (i * 10 - count) * (i * 10 - count) / 300));
            send();
        }
        mBlank.reset();
    }
}
