#include "../hw/lis302dl.h"
#include "../hw/ws2801.h"
#include "../hw/ssd1306.h"
#include "../hw/tlc5940.h"
#include "../Power.h"
#include "../sdio.h"
#include "sdcard.h"
#include "eyes.h"

#include "carcontroller.h"

class SpiChip : public Spi::Chip
{
public:
    SpiChip(Spi& spi, Gpio::Pin& s0, Gpio::Pin& s1, int value) :
        Spi::Chip(spi),
        mS0(s0),
        mS1(s1),
        mValue(value)
    { }

    void prepare() { mS0.set(mValue & 0x01); mS1.set(mValue & 0x02); }

private:
    Gpio::Pin& mS0;
    Gpio::Pin& mS1;
    int mValue;

};


CarController::CarController(SysTickControl &sysTick, HcSr04& distance, int distanceLeft, int distanceRight, int distanceFront,
                             CmdMotor &motor, unsigned steering, unsigned propulsion, unsigned measureDirection,
                             Tlc5940& light) :
    mState(Stop),
    mCounter(0),
    mTimer(*this, 100),
    mDistance(distance),
    mDistanceLeft(distanceLeft),
    mDistanceRight(distanceRight),
    mDistanceFront(distanceFront),
    mMotor(motor),
    mSteering(steering),
    mPropulsion(propulsion),
    mMeasureDirection(measureDirection),
    mLight(light),
    mEyes(nullptr),
    mRunning(false),
    mSpeed(0),
    mDestinationSpeed(0)
{
    sysTick.addRepeatingEvent(&mTimer);
    mLastDistanceIndex[0] = mDistanceFront;
    mLastDistanceIndex[1] = CENTER;
}


void CarController::eventCallback(System::Event *event)
{
    static const int ACCELERATION_FACTOR = 1;
    if (event == &mTimer && mRunning)
    {
        newPosition(mDistance.distance(mLastDistanceIndex[0]), mLastDistanceIndex[1]);
        switch (mCounter % 4)
        {
        case 0: mLastDistanceIndex[0] = mDistanceFront; mLastDistanceIndex[1] = CENTER; break;
        case 1: mLastDistanceIndex[0] = mDistanceLeft; mLastDistanceIndex[1] = LEFT; break;
        case 2: mLastDistanceIndex[0] = mDistanceFront; mLastDistanceIndex[1] = CENTER; break;
        case 3: mLastDistanceIndex[0] = mDistanceRight; mLastDistanceIndex[1] = RIGHT; break;
        }
        mDistance.start(mLastDistanceIndex[0]);
        State old = mState;
        if (mCounter < 20)
        {
            look(0);
            steer(0);
            mState = Stop;
            if ((mCounter % 10) == 0)
            {
                if (mEyes != nullptr) mEyes->start(Eyes::Blink, Eyes::Blink);
            }
        }
        else
        {
            switch (mState)
            {
            case CarController::Stop:
                setSpeed(0);
                steer(0);
                look(0);
                if (position(CENTER) > 1000) mState = Forward;
                break;
            case CarController::Forward:
                if (mSpeed < 30)
                {
                    mSpeed += ACCELERATION_FACTOR;
                }
                else
                {
                    if (position(CENTER) < 800)
                    {
                        mSpeed = 50;
                        if (position(LEFT) > position(RIGHT))
                        {
                            steer(80);
                            mState = SteerLeft;
                        }
                        else
                        {
                            steer(-80);
                            mState = SteerRight;
                        }
                    }
                    else
                    {
                        setBlink(true, false);
                        if (mSpeed < 100) mSpeed += ACCELERATION_FACTOR;
                        look((static_cast<int>(mCounter % 41) - 20));
    //                    int delta = position(LEFT) - position(RIGHT);
    //                    if (delta < 100 && delta > -100) delta = 0;
    //                    else if (delta >= 100) delta = 20;
    //                    else if (delta <= 100) delta = -20;
    //                    steer(delta);
                        steer(0);
                    }
                }
                break;
            case CarController::SteerLeft:
                look((static_cast<int>(mCounter % 41) + 10));
                setBlink(true, (mCounter % 10) < 5);
                if (position(CENTER) > 1000)
                {
                    mState = Forward;
                }
                else if (position(LEFT) < 200 || position(CENTER) < 400 || position(RIGHT) < 200)
                {
                    mState = ReverseRight;
                    steer(-80);
                    look(-80);
                    mSpeed = 0;
                }
                break;
            case CarController::SteerRight:
                look((static_cast<int>(mCounter % 41) - 50));
                setBlink(false, (mCounter % 10) < 5);
                if (position(CENTER) > 1000)
                {
                    mState = Forward;
                }
                else if (position(LEFT) < 200 || position(CENTER) < 400 || position(RIGHT) < 200)
                {
                    mState = ReverseLeft;
                    steer(80);
                    look(80);
                    mSpeed = 0;
                }
                break;
            case CarController::ReverseLeft:
                setBlink(true, (mCounter % 10) < 5);
                if (mSpeed > -50)
                {
                    mSpeed -= ACCELERATION_FACTOR;
                }
                else
                {
                    if (mSpeed > -70) mSpeed -= ACCELERATION_FACTOR;
                    if (position(RIGHT) > 1000)
                    {
                        mSpeed = 0;
                        mState = Forward;
                        steer(0);
                        look(0);
                    }
                    else if (position(LEFT) < 200 || position(CENTER) < 200)
                    {
                        mCounter = 0;
                        mState = Stop;
                    }
                }
                break;
            case CarController::ReverseRight:
                setBlink(false, (mCounter % 10) < 5);
                if (mSpeed > -50)
                {
                    mSpeed -= ACCELERATION_FACTOR;
                }
                else
                {
                    if (mSpeed > -70) mSpeed -= ACCELERATION_FACTOR;
                    if (position(LEFT) > 1000)
                    {
                        mSpeed = 0;
                        mState = Forward;
                        steer(0);
                        look(0);
                    }
                    else if (position(CENTER) < 200 || position(RIGHT) < 200)
                    {
                        mCounter = 0;
                        mState = Stop;
                    }
                }
                break;

            }
        }
        setSpeed(mSpeed);
        if ((mCounter % 5) == 0) mLight.send();

        ++mCounter;
        if (old != mState || (mCounter % 10) == 0)
        {
            printf("%i: %lu,%lu,%lu\n", mState, position(LEFT), position(CENTER), position(RIGHT));
        }
//        if ((mCounter % 10) == 0)
//        {
//            for (int p = 0; p < 3; ++p)
//            {
//                for (int w = 0; w < 3; ++w)
//                {
//                    //for (int i = HISTORY_SIZE - 1; i >= 0; --i)
//                    {
//                        printf("%6i ", mParam[w][p][HISTORY_SIZE]);
//                    }
//                }
//                printf("\r\n");
//            }
//            printf("\r\n");
//        }
    }
}

bool CarController::steer(int left)
{
//    return true;
    return mMotor.set(mSteering, -left);
}

bool CarController::setSpeed(int forward)
{
//    return true;
    mSpeed = forward;
    return mMotor.set(mPropulsion, forward);
}

bool CarController::look(int left)
{
//    return true;
    return mMotor.set(mMeasureDirection, -left);
}

void CarController::newPosition(int position, int which)
{
    for (int physic = 0; physic < 3; ++physic)
    {
        int sum = 0;
        for (int i = HISTORY_SIZE - 1; i > 0; --i)
        {
            mParam[which][physic][i] = mParam[which][physic][i - 1];
            sum += mParam[which][physic][i];
        }
        if (physic == 0)
        {
            mParam[which][physic][0] = std::min<uint32_t>(4000, position);
        }
        else
        {
            mParam[which][physic][0] = mParam[which][physic - 1][0] - mParam[which][physic - 1][1];
        }
        sum += mParam[which][physic][0];
        mParam[which][physic][HISTORY_SIZE] = sum / HISTORY_SIZE;
    }
}

void CarController::setBlink(bool left, bool on)
{
    if (mEyes != nullptr)
    {
        if (on)
        {
            if (left) mEyes->start(Eyes::LookLeft, Eyes::LookLeft);
            else mEyes->start(Eyes::LookRight, Eyes::LookRight);
        }
        else mEyes->start(Eyes::Open, Eyes::Open);
    }

    mLight.setOutput(RIGHT_BLINK_BACK_LIGHT, !left ? (on ? 100 : 0) : 0);
    mLight.setOutput(RIGHT_SIDE_BOTTOM_LIGHT, !left ? (on ? 100 : 0) : 0);
    mLight.setOutput(LEFT_BLINK_BACK_LIGHT, left ? (on ? 100 : 0) : 0);
    mLight.setOutput(LEFT_SIDE_BOTTOM_LIGHT, left ? (on ? 100 : 0) : 0);
}

void CarController::start()
{
    mLight.setOutput(RIGHT_BACK_LIGHT, 100);
    mLight.setOutput(LEFT_BACK_LIGHT, 100);
    mLight.setOutput(RIGHT_FRONT_LEFT_LIGHT, 100);
    mLight.setOutput(RIGHT_FRONT_RIGHT_LIGHT, 100);
    mLight.setOutput(LEFT_FRONT_LEFT_LIGHT, 100);
    mLight.setOutput(LEFT_FRONT_RIGHT_LIGHT, 100);

    mLight.setOutput(RIGHT_REVERSE_LIGHT, 0);
    mLight.setOutput(LEFT_REVERSE_LIGHT, 0);
    mLight.setOutput(RIGHT_BREAK_LIGHT, 0);
    mLight.setOutput(LEFT_BREAK_LIGHT, 0);
    mLight.setOutput(RIGHT_BLINK_BACK_LIGHT, 0);
    mLight.setOutput(LEFT_BLINK_BACK_LIGHT, 0);
    mLight.setOutput(RIGHT_SIDE_TOP_LIGHT, 0);
    mLight.setOutput(LEFT_SIDE_TOP_LIGHT, 0);
    mLight.setOutput(RIGHT_SIDE_BOTTOM_LIGHT, 0);
    mLight.setOutput(LEFT_SIDE_BOTTOM_LIGHT, 0);
    mLight.send();

    mCounter = 0;
    mState = Stop;
    mRunning = true;
    if (mEyes != 0) mEyes->back();
}

void CarController::stop()
{
    mRunning = false;
    steer(0);
    setSpeed(0);
    mLight.setOutput(RIGHT_BACK_LIGHT, 0);
    mLight.setOutput(LEFT_BACK_LIGHT, 0);
    mLight.setOutput(RIGHT_FRONT_LEFT_LIGHT, 50);
    mLight.setOutput(RIGHT_FRONT_RIGHT_LIGHT, 0);
    mLight.setOutput(LEFT_FRONT_LEFT_LIGHT, 50);
    mLight.setOutput(LEFT_FRONT_RIGHT_LIGHT, 0);

    mLight.setOutput(RIGHT_REVERSE_LIGHT, 0);
    mLight.setOutput(LEFT_REVERSE_LIGHT, 0);
    mLight.setOutput(RIGHT_BREAK_LIGHT, 0);
    mLight.setOutput(LEFT_BREAK_LIGHT, 0);
    mLight.setOutput(RIGHT_BLINK_BACK_LIGHT, 0);
    mLight.setOutput(LEFT_BLINK_BACK_LIGHT, 0);
    mLight.setOutput(RIGHT_SIDE_TOP_LIGHT, 0);
    mLight.setOutput(LEFT_SIDE_TOP_LIGHT, 0);
    mLight.setOutput(RIGHT_SIDE_BOTTOM_LIGHT, 0);
    mLight.setOutput(LEFT_SIDE_BOTTOM_LIGHT, 0);
    mLight.send();

    look(0);
    if (mEyes != 0) mEyes->start(Eyes::Close, Eyes::Close);
}

void CarController::init(StmSystem &sys, CommandInterpreter& interpreter)
{
    // SPI1 for acceleration sensor and OLED displays
    sys.mRcc.enable(ClockControl::Function::Spi1);
    sys.mRcc.enable(ClockControl::Function::GpioA);
    sys.mRcc.enable(ClockControl::Function::GpioE);
    // SCK
    sys.mGpioA.configOutput(Gpio::Index::Pin5, Gpio::OutputType::PushPull, Gpio::Pull::Down);
    sys.mGpioA.setAlternate(Gpio::Index::Pin5, Gpio::AltFunc::SPI1);
    // MISO
    sys.mGpioA.configInput(Gpio::Index::Pin6);
    sys.mGpioA.setAlternate(Gpio::Index::Pin6, Gpio::AltFunc::SPI1);
    // MOSI
    sys.mGpioA.configOutput(Gpio::Index::Pin7, Gpio::OutputType::PushPull);
    sys.mGpioA.setAlternate(Gpio::Index::Pin7, Gpio::AltFunc::SPI1);
    // CS
    sys.mGpioE.configOutput(Gpio::Index::Pin3, Gpio::OutputType::PushPull);
    // INT1
    sys.mGpioE.configInput(Gpio::Index::Pin0);
    sys.mRcc.enable(ClockControl::Function::SysCfg);
    sys.mSysCfg.extIntSource(Gpio::Index::Pin0, SysCfg::Gpio::E);
    InterruptController::Line extInt0(sys.mNvic, StmSystem::InterruptIndex::EXTI0);
    extInt0.setCallback(&sys.mExtI);
    extInt0.enable();
    // INT2
    sys.mGpioE.configInput(Gpio::Index::Pin1);
    sys.mSysCfg.extIntSource(Gpio::Index::Pin1, SysCfg::Gpio::E);
    InterruptController::Line extInt1(sys.mNvic, StmSystem::InterruptIndex::EXTI1);
    extInt1.setCallback(&sys.mExtI);
    extInt1.enable();

    sys.mSpi1.configDma(new Dma::Stream(sys.mDma2, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel3,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA2_Stream3)),
                         new Dma::Stream(sys.mDma2, Dma::Stream::StreamIndex::Stream2, Dma::Stream::ChannelIndex::Channel3,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA2_Stream2))
                         );
    sys.mSpi1.setMasterSlave(Spi::MasterSlave::Master);
    sys.mSpi1.enable(Device::Part::All);


    // Motor command
//    gSys.mGpioE.configOutput(Gpio::Index::Pin8, Gpio::OutputType::PushPull);
//    gSys.mGpioE.configOutput(Gpio::Index::Pin10, Gpio::OutputType::PushPull);
//    gSys.mGpioE.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull);
//    gSys.mGpioE.configOutput(Gpio::Index::Pin14, Gpio::OutputType::PushPull);
    sys.mRcc.enable(ClockControl::Function::Tim1);
    Timer timer1(StmSystem::BaseAddress::TIM1, ClockControl::Clock::APB2);
    timer1.setFrequency(sys.mRcc, 500);
    timer1.configCompare(Timer::CaptureCompareIndex::Index1, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::Disabled, Timer::CompareOutput::ActiveHigh);
    timer1.configCompare(Timer::CaptureCompareIndex::Index2, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::Disabled, Timer::CompareOutput::ActiveHigh);
    timer1.configCompare(Timer::CaptureCompareIndex::Index3, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::Disabled, Timer::CompareOutput::ActiveHigh);
    timer1.configCompare(Timer::CaptureCompareIndex::Index4, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    timer1.setCompare(Timer::CaptureCompareIndex::Index2, 0);
    timer1.setCompare(Timer::CaptureCompareIndex::Index1, 0);
    timer1.setCompare(Timer::CaptureCompareIndex::Index3, 0);
    timer1.setCompare(Timer::CaptureCompareIndex::Index4, 0);
    timer1.enable();
    sys.mRcc.enable(ClockControl::Function::Tim3);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin0, Gpio::OutputType::PushPull);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin1, Gpio::OutputType::PushPull);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin4, Gpio::OutputType::PushPull);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin5, Gpio::OutputType::PushPull);
    Timer timer3(StmSystem::BaseAddress::TIM3, ClockControl::Clock::APB1);
    timer3.setFrequency(sys.mRcc, 500);
    timer3.configCompare(Timer::CaptureCompareIndex::Index1, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    timer3.configCompare(Timer::CaptureCompareIndex::Index2, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    timer3.configCompare(Timer::CaptureCompareIndex::Index3, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    timer3.configCompare(Timer::CaptureCompareIndex::Index4, Timer::CompareMode::PwmActiveWhenLower, Timer::CompareOutput::ActiveHigh, Timer::CompareOutput::Disabled);
    timer3.setCompare(Timer::CaptureCompareIndex::Index1, 0);
    timer3.setCompare(Timer::CaptureCompareIndex::Index2, 0);
    timer3.setCompare(Timer::CaptureCompareIndex::Index3, 0);
    timer3.setCompare(Timer::CaptureCompareIndex::Index4, 0);
    timer3.enable();
    CmdMotor* motor = new CmdMotor;
    motor->add(timer1, Timer::CaptureCompareIndex::Index1, Timer::CaptureCompareIndex::Index2);
    motor->add(timer1, Timer::CaptureCompareIndex::Index3, Timer::CaptureCompareIndex::Index4);
    motor->add(timer3, Timer::CaptureCompareIndex::Index1, Timer::CaptureCompareIndex::Index2);
    motor->add(timer3, Timer::CaptureCompareIndex::Index3, Timer::CaptureCompareIndex::Index4);
    interpreter.add(motor);
    sys.mRcc.enable(ClockControl::Function::GpioE);
    sys.mGpioE.setAlternate(Gpio::Index::Pin8, Gpio::AltFunc::TIM1);
    sys.mGpioE.setAlternate(Gpio::Index::Pin10, Gpio::AltFunc::TIM1);
    sys.mGpioE.setAlternate(Gpio::Index::Pin12, Gpio::AltFunc::TIM1);
    sys.mGpioE.setAlternate(Gpio::Index::Pin14, Gpio::AltFunc::TIM1);
    sys.mRcc.enable(ClockControl::Function::GpioB);
    sys.mGpioB.setAlternate(Gpio::Index::Pin0, Gpio::AltFunc::TIM3);
    sys.mGpioB.setAlternate(Gpio::Index::Pin1, Gpio::AltFunc::TIM3);
    sys.mGpioB.setAlternate(Gpio::Index::Pin4, Gpio::AltFunc::TIM3);
    sys.mGpioB.setAlternate(Gpio::Index::Pin5, Gpio::AltFunc::TIM3);


    Spi::Chip spi1(sys.mSpi1);
    // acceleration sensor
    LIS302DL lis(spi1);
    lis.configInterrupt(new ExternalInterrupt::Line(sys.mExtI, 0), new ExternalInterrupt::Line(sys.mExtI, 1));
    interpreter.add(new CmdLis(lis));

    // clock command
    InterruptController::Line timer11Irq(sys.mNvic, StmSystem::InterruptIndex::TIM1_TRG_COM_TIM11);
    Timer timer11(StmSystem::BaseAddress::TIM11, ClockControl::Clock::APB2);
    timer11.setInterrupt(Timer::InterruptType::CaptureCompare, &timer11Irq);
    Power pwr(StmSystem::BaseAddress::PWR);
    pwr.backupDomainWp(false);
    sys.mRcc.enableRtc(ClockControl::RtcClock::HighSpeedExternal);
    pwr.backupDomainWp(true);
    interpreter.add(new CmdMeasureClock(sys.mRcc, timer11));

    // pin command
    Gpio* gpio[] = { &sys.mGpioA, &sys.mGpioB, &sys.mGpioC, &sys.mGpioD, &sys.mGpioE, &sys.mGpioF, &sys.mGpioG, &sys.mGpioH, &sys.mGpioI };
    interpreter.add(new CmdPin(gpio, sizeof(gpio) / sizeof(gpio[0])));

    // rgb command
    sys.mRcc.enable(ClockControl::Function::Spi2);
    sys.mRcc.enable(ClockControl::Function::GpioB);
    // SCK
    sys.mGpioB.configOutput(Gpio::Index::Pin13, Gpio::OutputType::PushPull);
    sys.mGpioB.setAlternate(Gpio::Index::Pin13, Gpio::AltFunc::SPI2);
    // MISO
    sys.mGpioB.setAlternate(Gpio::Index::Pin14, Gpio::AltFunc::SPI2);
    // MOSI
    sys.mGpioB.configOutput(Gpio::Index::Pin15, Gpio::OutputType::PushPull);
    sys.mGpioB.setAlternate(Gpio::Index::Pin15, Gpio::AltFunc::SPI2);

    sys.mSpi2.configDma(new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream4, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream4)),
                         new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream3))
                         );

    // 74HC4052: 1x SPI3 -> 4x SPI
    sys.mRcc.enable(ClockControl::Function::GpioE);
    Gpio::Pin s0(sys.mGpioE, Gpio::Index::Pin5);
    Gpio::Pin s1(sys.mGpioE, Gpio::Index::Pin3);
    sys.mGpioE.configOutput(Gpio::Index::Pin5, Gpio::OutputType::PushPull);
    sys.mGpioE.configOutput(Gpio::Index::Pin3, Gpio::OutputType::PushPull);
    s0.reset();
    s1.reset();
    SpiChip spi3_0(sys.mSpi3, s0, s1, 0);
    SpiChip spi3_1(sys.mSpi3, s0, s1, 1);
    SpiChip spi3_2(sys.mSpi3, s0, s1, 2);
    SpiChip spi3_3(sys.mSpi3, s0, s1, 3);


    // 2 OLED displays
    sys.mRcc.enable(ClockControl::Function::GpioC);
    Gpio::Pin dataCommand(sys.mGpioC, Gpio::Index::Pin9);
    Gpio::Pin cs1(sys.mGpioC, Gpio::Index::Pin7);
    Gpio::Pin cs2(sys.mGpioC, Gpio::Index::Pin6);
    Gpio::Pin reset(sys.mGpioC, Gpio::Index::Pin8);
    sys.mGpioC.configOutput(Gpio::Index::Pin6, Gpio::OutputType::PushPull);
    sys.mGpioC.configOutput(Gpio::Index::Pin7, Gpio::OutputType::PushPull);
    sys.mGpioC.configOutput(Gpio::Index::Pin8, Gpio::OutputType::PushPull);
    sys.mGpioC.configOutput(Gpio::Index::Pin9, Gpio::OutputType::PushPull);
    Ssd1306 oled1(spi1, cs1, dataCommand, reset);
    Ssd1306 oled2(spi1, cs2, dataCommand, reset);
    oled2.reset();
    //oled2.reset(); // Same reset as oled1
    oled1.drawString(0, 0, "Hello World 1!");
    oled1.init();
    oled2.drawString(0, 0, "Hello World 2!");
    oled2.init();


    Eyes eyes(sys.mSysTick, &oled1, &oled2);
    eyes.start(Eyes::Blink, Eyes::Blink);

    // SPI3
    sys.mRcc.enable(ClockControl::Function::GpioC);
    // SCK
    sys.mGpioC.configOutput(Gpio::Index::Pin10, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    sys.mGpioC.setAlternate(Gpio::Index::Pin10, Gpio::AltFunc::SPI3);
    // MISO
    sys.mGpioC.configInput(Gpio::Index::Pin11);
    sys.mGpioC.setAlternate(Gpio::Index::Pin11, Gpio::AltFunc::SPI3);
    // MOSI
    sys.mGpioC.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    sys.mGpioC.setAlternate(Gpio::Index::Pin12, Gpio::AltFunc::SPI3);

    sys.mRcc.enable(ClockControl::Function::Spi3);
    sys.mSpi3.configDma(new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream5, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream5)),
                         new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream0, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream0))
                         );
    sys.mSpi3.setMasterSlave(Spi::MasterSlave::Master);
    sys.mSpi3.enable(Device::Part::All);


    // LED command
    // TLC5940: 16x PWM LED
    sys.mRcc.enable(ClockControl::Function::GpioD);
    sys.mRcc.enable(ClockControl::Function::GpioB);
    sys.mRcc.enable(ClockControl::Function::Tim9);
    sys.mRcc.enable(ClockControl::Function::Tim10);
    Gpio::Pin xlat(sys.mGpioD, Gpio::Index::Pin1);
    Gpio::Pin blank(sys.mGpioD, Gpio::Index::Pin3);
    //Gpio::Pin gsclk(gSys.mGpioB, Gpio::Index::Pin8);
    sys.mGpioD.configOutput(Gpio::Index::Pin1, Gpio::OutputType::PushPull);
    sys.mGpioD.configOutput(Gpio::Index::Pin3, Gpio::OutputType::PushPull);
    sys.mGpioB.configOutput(Gpio::Index::Pin8, Gpio::OutputType::PushPull);
    sys.mGpioB.setAlternate(Gpio::Index::Pin8, Gpio::AltFunc::TIM10);
    Timer gsclkPwm(StmSystem::BaseAddress::TIM10, ClockControl::Clock::APB2);
    gsclkPwm.setFrequency(sys.mRcc, 4096 * 50);
    Timer gsclkLatch(StmSystem::BaseAddress::TIM9, ClockControl::Clock::APB2);
    InterruptController::Line timer9IrqUpdate(sys.mNvic, StmSystem::InterruptIndex::TIM1_BRK_TIM9);
    gsclkLatch.setInterrupt(Timer::InterruptType::Update, &timer9IrqUpdate);
    Tlc5940 tlc(spi3_0, xlat, blank, gsclkPwm, gsclkLatch);
    interpreter.add(new CmdLed(tlc));

    Ws2801 ws(spi3_1, 1);
    ws.enable();
    interpreter.add(new CmdRgb(ws));


    sys.mRcc.enable(ClockControl::Function::GpioC);
    sys.mGpioC.configInput(Gpio::Index::Pin1, Gpio::Pull::Up);


//    gSys.mRcc.enable(ClockControl::Function::Sdio);
//    gSys.mRcc.enable(ClockControl::Function::GpioC);
//    gSys.mRcc.enable(ClockControl::Function::GpioD);
//    gSys.mGpioC.configOutput(Gpio::Index::Pin8, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Fast);    // D0
//    gSys.mGpioC.configOutput(Gpio::Index::Pin9, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Fast);    // D1
//    gSys.mGpioC.configOutput(Gpio::Index::Pin10, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Fast);    // D2
//    gSys.mGpioC.configOutput(Gpio::Index::Pin11, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Fast);    // D3
//    gSys.mGpioC.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);    // CK
//    gSys.mGpioD.configOutput(Gpio::Index::Pin2, Gpio::OutputType::PushPull, Gpio::Pull::Up, Gpio::Speed::Fast);    // CMD
//    gSys.mGpioC.setAlternate(Gpio::Index::Pin8, Gpio::AltFunc::SDIO);    // D0
//    gSys.mGpioC.setAlternate(Gpio::Index::Pin9, Gpio::AltFunc::SDIO);    // D1
//    gSys.mGpioC.setAlternate(Gpio::Index::Pin10, Gpio::AltFunc::SDIO);    // D2
//    gSys.mGpioC.setAlternate(Gpio::Index::Pin11, Gpio::AltFunc::SDIO);    // D3
//    gSys.mGpioC.setAlternate(Gpio::Index::Pin12, Gpio::AltFunc::SDIO);    // CK
//    gSys.mGpioD.setAlternate(Gpio::Index::Pin2, Gpio::AltFunc::SDIO);    // CMD

//    gSys.mRcc.enable(ClockControl::Function::Dma2);
//    InterruptController::Line sdioIrq(gSys.mNvic, StmSystem::InterruptIndex::SDIO);
//    Dma::Stream sdioDma(gSys.mDma2, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel4,
//        new InterruptController::Line(gSys.mNvic, StmSystem::InterruptIndex::DMA2_Stream3));

//    Sdio sdio(StmSystem::BaseAddress::SDIO, sdioIrq, sdioDma);
//    SdCard sdCard(sdio, 30);
//    interpreter.add(new CmdSdio(sdCard));

    // 4 x GPIO
    // Hall sensor needs pullup
    sys.mGpioB.configInput(Gpio::Index::Pin11, Gpio::Pull::Up);
    sys.mGpioE.configInput(Gpio::Index::Pin11, Gpio::Pull::Up);
    sys.mGpioE.configInput(Gpio::Index::Pin13, Gpio::Pull::Up);
    sys.mGpioE.configInput(Gpio::Index::Pin15, Gpio::Pull::Up);

    Gpio::ConfigurablePin gpio0(sys.mGpioB, Gpio::Index::Pin11);
    Gpio::ConfigurablePin gpio1(sys.mGpioE, Gpio::Index::Pin15);
    Gpio::ConfigurablePin gpio2(sys.mGpioE, Gpio::Index::Pin13);
    Gpio::ConfigurablePin gpio3(sys.mGpioE, Gpio::Index::Pin11);

    sys.mSysCfg.extIntSource(Gpio::Index::Pin11, SysCfg::Gpio::E);
    sys.mSysCfg.extIntSource(Gpio::Index::Pin13, SysCfg::Gpio::E);
    sys.mSysCfg.extIntSource(Gpio::Index::Pin15, SysCfg::Gpio::E);
    InterruptController::Line extInt10_15(sys.mNvic, StmSystem::InterruptIndex::EXTI15_10);
    extInt10_15.setCallback(&sys.mExtI);
    extInt10_15.enable();
    HcSr04 hc(sys.mSysTick, gpio1, new ExternalInterrupt::Line(sys.mExtI, 15));
    hc.addDevice(gpio2, new ExternalInterrupt::Line(sys.mExtI, 13));
    hc.addDevice(gpio3, new ExternalInterrupt::Line(sys.mExtI, 11));
    CmdDistance dist(hc);
    interpreter.add(&dist);

    CarController cc(sys.mSysTick, hc, 0, 1, 2,
                     *motor, 1, 0, 3,
                     tlc);
    cc.setEyes(&eyes);
    cc.stop();

}
