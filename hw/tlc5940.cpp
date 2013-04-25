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
    mSpi.setSpeed(1 * 1000* 1000);
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
    static int count = 0;
    static int inc = 1;
    static int table[] = { 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 8, 9, 11, 12, 14, 16, 18, 20, 23, 25, 28, 30, 33, 36, 39, 42, 46, 49, 53, 56, 60, 64, 68, 72, 77, 81, 86, 90, 95, 100, 105, 110, 116, 121, 127, 132, 138, 144, 150, 156, 163, 169, 176, 182, 189, 196, 203, 210, 218, 225, 233, 240, 248, 256, 264, 272, 281, 289, 298, 306, 315, 324, 333, 342, 352, 361, 371, 380, 390, 400, 410, 420, 431, 441, 452, 462, 473, 484, 495, 506, 518, 529, 541, 552, 564, 576, 588, 600, 613, 625, 638, 650, 663, 676, 689, 702, 716, 729, 743, 756, 770, 784, 798, 812, 827, 841, 856, 870, 885, 900, 915, 930, 946, 961, 977, 992, 1008, 1024, 1040, 1056, 1073, 1089, 1106, 1122, 1139, 1156, 1173, 1190, 1208, 1225, 1243, 1260, 1278, 1296, 1314, 1332, 1351, 1369, 1388, 1406, 1425, 1444, 1463, 1482, 1502, 1521, 1541, 1560, 1580, 1600, 1620, 1640, 1661, 1681, 1702, 1722, 1743, 1764, 1785, 1806, 1828, 1849, 1871, 1892, 1914, 1936, 1958, 1980, 2003, 2025, 2048, 2070, 2093, 2116, 2139, 2162, 2186, 2209, 2233, 2256, 2280, 2304, 2328, 2352, 2377, 2401, 2426, 2450, 2475, 2500, 2525, 2550, 2576, 2601, 2627, 2652, 2678, 2704, 2730, 2756, 2783, 2809, 2836, 2862, 2889, 2916, 2943, 2970, 2998, 3025, 3053, 3080, 3108, 3136, 3164, 3192, 3221, 3249, 3278, 3306, 3335, 3364, 3393, 3422, 3452, 3481, 3511, 3540, 3570, 3600, 3630, 3660, 3691, 3721, 3752, 3782, 3813, 3844, 3875, 3906, 3938, 3969, 4001, 4032, 4095 };
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
            if (count >= 170 || count <= 0) inc = -inc;
            //for (int i = 1; i <= 16; ++i) setOutput(i - 1, std::max(0, 4095 - 4096 * (i * 10 - count) * (i * 10 - count) / 300));
            for (int i = 1; i <= 16; ++i) setOutput(i - 1, table[std::max(0, std::min(255, 255 - 255 * (i * 10 - count) * (i * 10 - count) / 100))]);
            //for (int i = 1; i <= 16; ++i) setOutput(i - 1, 4095);
            send();
        }
        mBlank.reset();
    }
}
