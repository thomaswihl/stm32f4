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

void Tlc5940::setOutput(int index, int percent)
{
    static int table[] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 15, 16, 17, 19, 21, 23, 25, 27, 29, 32, 35, 38, 41, 45, 49, 53, 58, 63, 69, 75, 81, 88, 96, 104, 114, 123, 134, 146, 159, 173, 188, 204, 222, 241, 262, 285, 310, 337, 366, 398, 433, 470, 511, 555, 604, 656, 713, 775, 842, 916, 995, 1081, 1175, 1277, 1388, 1509, 1640, 1782, 1937, 2105, 2287, 2486, 2701, 2936, 3190, 3467, 3768, 4095 };
    if (index < 0 || index > 15) return;

    if (percent < 0) percent = 0;
    else if (percent >= static_cast<int>(sizeof(table) / sizeof(table[0]))) percent = sizeof(table) / sizeof(table[0]) - 1;
    int value = table[percent];
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
