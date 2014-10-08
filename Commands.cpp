#include "Commands.h"
#include "hw/ds18b20.h"

#include <cmath>
#include <strings.h>

char const * const CmdHelp::NAME[] = { "help", "?" };
char const * const CmdHelp::ARGV[] = { "os:command" };

char const * const CmdInfo::NAME[] = { "info" };
char const * const CmdInfo::ARGV[] = { nullptr };

char const * const CmdFunc::NAME[] = { "func" };
char const * const CmdFunc::ARGV[] = { "s:function" };

char const * const CmdRead::NAME[] = { "read", "rb", "rh", "rw" };
char const * const CmdRead::ARGV[] = { "Au:address", "Vou:count" };

char const * const CmdWrite::NAME[] = { "write", "wb", "wh", "ww" };
char const * const CmdWrite::ARGV[] = { "Au:address", "Vu:data" };

char const * const CmdLis::NAME[] = { "lis" };
char const * const CmdLis::ARGV[] = { nullptr };

char const * const CmdPin::NAME[] = { "pin" };
char const * const CmdPin::ARGV[] = { "Ps:pin", "Vob:value" };

char const * const CmdMeasureClock::NAME[] = { "clock" };
char const * const CmdMeasureClock::ARGV[] = { nullptr };

char const * const CmdRgb::NAME[] = { "rgb" };
char const * const CmdRgb::ARGV[] = { "Vu:index", "Vu:red", "Vu:green", "Vu:blue" };

char const * const CmdLightSensor::NAME[] = { "ls" };
char const * const CmdLightSensor::ARGV[] = { nullptr };

char const * const CmdSdio::NAME[] = { "sd" };
char const * const CmdSdio::ARGV[] = { "s:command" };

char const * const CmdMotor::NAME[] = { "motor" };
char const * const CmdMotor::ARGV[] = { "u:index", "i:speed" };

char const * const CmdLed::NAME[] = { "led" };
char const * const CmdLed::ARGV[] = { "u:index", "u:brightness" };

char const * const CmdDistance::NAME[] = { "dist" };
char const * const CmdDistance::ARGV[] = { nullptr };

char const * const CmdSpiTest::NAME[] = { "spi" };
char const * const CmdSpiTest::ARGV[] = { "u:speed" };


CmdHelp::CmdHelp() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdHelp::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    char const * argCmd = nullptr;
    unsigned int argCmdLen = 0;
    if (argc == 2)
    {
        argCmd = argv[1].value.s;
        argCmdLen = strlen(argv[1].value.s);
    }
    for (auto i : interpreter)
    {
        CommandInterpreter::Command* cmd = i;
        if (argc != 2 || cmd->startsWith(argCmd, argCmdLen) != nullptr)
        {
            interpreter.printAliases(cmd);
            printf(" ");
            interpreter.printArguments(cmd, true);
            printf("\n");
            interpreter.printArguments(cmd, false);
            printf("  %s\n\n", cmd->helpText());
        }
    }
    return true;
}

CmdRead::CmdRead() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdRead::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    unsigned int count = 1;
    if (argc == 3) count = argv[2].value.u;
    switch (argv[0].value.s[1])
    {
    case 'b':
        dump<uint8_t>(reinterpret_cast<uint8_t*>(argv[1].value.u), count);
        break;
    case 'h':
        dump<uint16_t>(reinterpret_cast<uint16_t*>(argv[1].value.u), count);
        break;
    default:
        dump<uint32_t>(reinterpret_cast<uint32_t*>(argv[1].value.u), count);
        break;
    }
    return true;
}

template<class T>
void CmdRead::dump(T* address, unsigned int count)
{
    char format[16];
    std::sprintf(format, " %%0%ux", sizeof(T) * 2);
    T* p = address;
    for (unsigned int i = 0; i < count; ++i)
    {
        if ((i % (32 / sizeof(T))) == 0)
        {
            if (i != 0) printf("\n");
            printf("%08x:", reinterpret_cast<unsigned int>(p));
        }
        printf(format, *p++);
    }
    printf("\n");
}

CmdWrite::CmdWrite() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdWrite::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    switch (argv[0].value.s[1])
    {
    case 'b':
        *reinterpret_cast<uint8_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    case 'h':
        *reinterpret_cast<uint16_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    default:
        *reinterpret_cast<uint32_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    }
    return true;
}


CmdInfo::CmdInfo(StmSystem &system) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSystem(system)
{
}

bool CmdInfo::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    mSystem.printInfo();
    return true;
}


CmdFunc::CmdFunc(StmSystem &system) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSystem(system)
{
}

bool CmdFunc::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    if (strcmp("ns", argv[1].value.s) == 0)
    {
        printf("%llu\n", mSystem.ns());
    }
    else if (strcmp("temp", argv[1].value.s) == 0)
    {
        mSystem.mRcc.enable(ClockControl::Function::GpioB);
        Gpio::Pin pin(mSystem.mGpioE, Gpio::Index::Pin11);
        mSystem.mGpioB.configOutput(Gpio::Index::Pin11, Gpio::OutputType::OpenDrain, Gpio::Pull::Up);
        Ds18b20 temp(pin);
        int t = temp.temp();
        printf("%u.%u\n", t / 16, 1000 * (t % 16) / 16);
    }
    else
    {
        printf("Unknown function, allowed is: ns\n");
    }
    return true;
}


CmdLis::CmdLis(LIS302DL &lis) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mLis(lis), mEvent(*this), mEnabled(false)
{
    mLis.setDataReadyEvent(&mEvent);
}

bool CmdLis::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    if (mEnabled)
    {
        mLis.disable();
        mEnabled = false;
    }
    else
    {
        mLis.enable();
        (void)mLis.x();
        (void)mLis.y();
        (void)mLis.z();
        mEnabled = true;
    }
    return true;
}

void CmdLis::eventCallback(System::Event *event)
{
    int x, y, z;
    x = mLis.x();
    y = mLis.y();
    z = mLis.z();
    float a = x * x + y * y + z * z;
    a = std::sqrt(a / 2500);
    printf("\x1b[s\x1b[2;H%3i %3i %3i = %.2fg\x1b[K\x1b[u", x, y, z, a);
    fflush(nullptr);
}


CmdPin::CmdPin(Gpio **gpio, unsigned int gpioCount) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mGpio(gpio), mGpioCount(gpioCount)
{
}

bool CmdPin::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    if (argc == 2 && strcasecmp(argv[1].value.s, "all") == 0)
    {
        for (unsigned int index = 0; index < mGpioCount; ++index)
        {
            printf("GPIO %c: ", 'A' + index);
            for (int pin = 15; pin >= 0; --pin)
            {
                printf("%c", mGpio[index]->get(static_cast<Gpio::Index>(pin)) ? '1' : '0');
            }
            printf("\n");
        }
        return true;
    }
    char c = argv[1].value.s[0];
    unsigned int index = c - 'A';
    if (index > mGpioCount) index = c - 'a';
    if (index > mGpioCount)
    {
        printf("Invalid GPIO, GPIO A-%c (or a-%c) is allowed.\n", 'A' + mGpioCount - 1, 'a' + mGpioCount - 1);
        return false;
    }
    char* end;
    unsigned int pin = strtoul(argv[1].value.s +1, &end, 10);
    if (*end != 0 || pin > 15)
    {
        printf("Invalid index, GPIO has only 16 pins, so 0-15 is allowed.\n");
        return false;
    }
    if (argc == 2)
    {
        printf("GPIO %c%i = %c\n", 'A' + index, pin, mGpio[index]->get(static_cast<Gpio::Index>(pin)) ? '1' : '0');
    }
    else
    {
        if (argv[2].value.b) mGpio[index]->set(static_cast<Gpio::Index>(pin));
        else mGpio[index]->reset(static_cast<Gpio::Index>(pin));
    }
    return true;
}


CmdMeasureClock::CmdMeasureClock(ClockControl &clockControl, Timer &timer) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mClockControl(clockControl), mTimer(timer), mEvent(*this), mCount(0)
{
}

bool CmdMeasureClock::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    mClockControl.enable(ClockControl::Function::Tim11);
    mClockControl.setPrescaler(ClockControl::RtcHsePrescaler::by16);  // HSE_RTC = HSE / 16 = 500kHz
    // Timer 11 capture input = HSE_RTC / 8 = 62500Hz
    mTimer.configCapture(Timer::CaptureCompareIndex::Index1, Timer::Prescaler::Every8, Timer::Filter::F1N1, Timer::CaptureEdge::Rising);
    // So with HSI = 16MHz -> TIM_CLK = 32MHz this should give us 512 counts
    mTimer.setOption(Timer::Option::Timer11_Input1_Hse_Rtc);
    mTimer.setEvent(Timer::EventType::CaptureCompare1, &mEvent);
    mTimer.enable();
    //for (int i = 0; i < 5; ++i)
    {
        uint32_t first = mTimer.capture(Timer::CaptureCompareIndex::Index1);
        uint32_t second;
        do
        {
            second = mTimer.capture(Timer::CaptureCompareIndex::Index1);
        }   while (second == first);
        uint32_t delta;
        if (second > first) delta = second - first;
        else delta = 65536 + second - first;
        printf("External high speed clock (HSE) is approximatly %luMHz (TIM11 CC = %lu).\n", 16 * 16 * 8 / delta, delta);
    }
    return true;
}

void CmdMeasureClock::eventCallback(System::Event *event)
{
    ++mCount;
    if (mCount > 8)
    {
        mTimer.disable();
        mCount = 0;
    }
    printf("%lu\n", mTimer.capture(Timer::CaptureCompareIndex::Index1));
}


CmdRgb::CmdRgb(Ws2801 &ws) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mWs(ws)
{
}

bool CmdRgb::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    mWs.set(argv[1].value.u, argv[2].value.u, argv[3].value.u, argv[4].value.u);
    return true;
}


CmdLightSensor::CmdLightSensor(Gpio::Pin &led, Gpio::Pin &s2, Gpio::Pin &s3, Timer &timer, Ws2801 &ws) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mLed(led), mS2(s2), mS3(s3), mTimer(timer), mWs(ws), mEvent(*this)
{
}

bool CmdLightSensor::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    setColor(Color::Red);
    mLed.set();
    mTimer.configCapture(Timer::CaptureCompareIndex::Index1, Timer::Prescaler::EveryEdge, Timer::Filter::F1N1, Timer::CaptureEdge::Rising);
    mTimer.setEvent(Timer::EventType::CaptureCompare1, &mEvent);
    mTimer.setPrescaler(10);
    mTimer.enable();
    return true;
}

void CmdLightSensor::eventCallback(System::Event *event)
{
    static int count = 0;
    static int lastValue = 0;
    static int lastDelta = 0;
    static Color color = Color::Red;
    static int r, g, b, w;
    static int rd = 0, gd = 0, bd = 0, wd = 0;
    static int rc, gc, bc, wc;


    ++count;
    int delta;
    int thisValue = mTimer.capture(Timer::CaptureCompareIndex::Index1);
    if (thisValue >= lastValue) delta = thisValue - lastValue;
    else delta = 65536 + thisValue - lastValue;
    if (count > 20 || (delta > 5 && std::abs(delta - lastDelta) < (delta / 200 + 5)))
    {
        switch (color)
        {
        case Color::Red: r = delta; rc = count; break;
        case Color::Green: g = delta; gc = count; break;
        case Color::Blue: b = delta; bc = count; break;
        case Color::White: w = delta; wc = count; break;
        }
        color = nextColor(color);
        if (color == Color::Red)
        {
            if (rd == 0 && gd == 0 && bd == 0 && wd == 0)
            {
                rd = r;
                gd = g;
                bd = b;
                wd = w;
            }
            else
            {
                mTimer.disable();
                r = rd - r;
                g = gd - g;
                b = bd - b;
                w = wd - w;
                printf("r = %5i, g = %5i, b = %5i, w = %5i, rc = %2i, gc = %2i, bc = %2i, wc = %2i\n", r, g, b, w, rc, gc, bc, wc);
                r = r / 2;
                g = g / 2;
                b = b / 2;
                r = std::min(255, std::max(r, 0));
                g = std::min(255, std::max(g, 0));
                b = std::min(255, std::max(b, 0));
                mWs.set(0, r, g, b);
                mLed.reset();
            }
        }
        setColor(color);
        count = 0;
        lastValue = lastDelta = 0;
    }
    lastValue = thisValue;
    lastDelta = delta;
}

void CmdLightSensor::setColor(Color color)
{
    switch (color)
    {
    case Color::Red:
        mS2.set(false);
        mS3.set(false);
        break;
    case Color::Green:
        mS2.set(true);
        mS3.set(true);
        break;
    case Color::Blue:
        mS2.set(false);
        mS3.set(true);
        break;
    case Color::White:
        mS2.set(true);
        mS3.set(false);
        break;
    }
}

CmdLightSensor::Color CmdLightSensor::nextColor(CmdLightSensor::Color color)
{
    switch (color)
    {
    case Color::Red: return Color::Green;
    case Color::Green: return Color::Blue;
    case Color::Blue: return Color::White;
    case Color::White: return Color::Red;
    }
    return Color::Red;
}


CmdSdio::CmdSdio(SdCard &sdCard) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSdCard(sdCard), mEvent(*this)
{
}

bool CmdSdio::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    if (strcmp("init", argv[1].value.s) == 0) mSdCard.init();
    else printf("Unknown command.");

    printf("\n");
    return true;
}

void CmdSdio::eventCallback(System::Event *event)
{
}


CmdMotor::CmdMotor() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mEvent(*this), mMotorCount(0)
{
}

bool CmdMotor::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    unsigned i = argv[1].value.u;
    int v = argv[2].value.i;
    return set(i, v);
}

bool CmdMotor::add(Timer &timer, Timer::CaptureCompareIndex m1, Timer::CaptureCompareIndex m2)
{
    if (mMotorCount >= sizeof(mMotor) / sizeof(mMotor[0])) return false;
    mMotor[mMotorCount].mTimer = &timer;
    mMotor[mMotorCount].mPin1 = m1;
    mMotor[mMotorCount].mPin2 = m2;
    ++mMotorCount;
    set(mMotorCount - 1, 0);
    return true;
}

bool CmdMotor::set(unsigned index, int value)
{
    if (index >= mMotorCount)
    {
        printf("Motor %u not available, index has to be in the range [0,%u].\n", index, mMotorCount - 1);
        return false;
    }
    if (value < -100 || value > 100)
    {
        printf("Motor value (%i) has to be in the range [-100, 100].\n", value);
        return false;
    }
    if (value < 0)
    {
        mMotor[index].mTimer->setCompare(mMotor[index].mPin1, -value * 65535 / 100);
        mMotor[index].mTimer->setCompare(mMotor[index].mPin2, 0);
    }
    else
    {
        mMotor[index].mTimer->setCompare(mMotor[index].mPin2, value * 65535 / 100);
        mMotor[index].mTimer->setCompare(mMotor[index].mPin1, 0);
    }
    return true;
}

void CmdMotor::eventCallback(System::Event *event)
{

}


CmdLed::CmdLed(Tlc5940 &pwm) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mEvent(*this), mPwm(pwm)
{
    for (int i = 0; i < 16; ++i) mPwm.setOutput(i, 0);
    mPwm.send();
}

bool CmdLed::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    unsigned i = argv[1].value.u;
    if (i >= 16)
    {
        printf("LED %u not available, index has to be in the range [0,15].\n", i);
        return false;
    }
    unsigned v = argv[2].value.u;
    if (v > 100)
    {
        printf("LED value has to be in the range [0, 100].\n");
        return false;
    }
    mPwm.setOutput(i, v);
    mPwm.send();
    return true;
}

void CmdLed::eventCallback(System::Event *event)
{

}


CmdDistance::CmdDistance(HcSr04& hc) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mHc(hc)
{
}

bool CmdDistance::execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv)
{
    mHc.start(0);
    return true;
}


CmdSpiTest::CmdSpiTest(Spi &spi, Gpio::Pin &ss) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSpi(spi), mSs(ss), mEvent(*this)
{
    memset(&mTransfer, 0, sizeof(mTransfer));
    mTransfer.mChip = nullptr;
    mTransfer.mChipSelect = this;
    mTransfer.mClockPhase = Spi::ClockPhase::FirstTransition;
    mTransfer.mClockPolarity = Spi::ClockPolarity::LowWhenIdle;
    mTransfer.mEndianess = Spi::Endianess::MsbFirst;
    mTransfer.mEvent = &mEvent;
    mTransfer.mLength = LEN;
    mTransfer.mReadData = new uint8_t[LEN];
    mWriteData = new uint8_t[LEN];
    mTransfer.mWriteData = mWriteData;
    for (unsigned i = 0; i < 16; ++i)
    {
        mWriteData[i] = i + (i << 4);
    }
    mSs.set();
}

bool CmdSpiTest::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    mTransfer.mMaxSpeed = argv[1].value.u;
    mSpi.transfer(&mTransfer);
    return true;
}

void CmdSpiTest::eventCallback(System::Event *event)
{
    unsigned failed = 0;
    printf("SPI: ");
    for (unsigned i = 0; i < LEN - 1; ++i)
    {
        //printf("%02x ", mTransfer.mReadData[i + 1]);
        if (mTransfer.mReadData[i + 1] != i)
        {
            printf("@%u ", i + 1);
            ++failed;
        }
    }
    printf(": %u failed\n", failed);
}


void CmdSpiTest::select()
{
    mSs.reset();
}

void CmdSpiTest::deselect()
{
    mSs.set();
}
