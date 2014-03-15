#include "hcsr04.h"

HcSr04::HcSr04(SysTickControl &sysTick, Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq) :
    mPin(pin), mIrq(irq), mEvent(*this), mState(Init), mDistanceIndex(0)
{
    mPin.configOutput(Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Medium);
}

void HcSr04::start()
{
    //printf("State = %i\n", mState);
    mState = SendStartPulse;
    mIrq->disable();
    mPin.setMode(Gpio::Mode::Output);
    mPin.set();
    System::instance()->usleep(20);
    mPin.setMode(Gpio::Mode::Input);
    mState = WaitForEchoStart;
    mIrq->setCallback(this);
    mIrq->enable(ExternalInterrupt::Trigger::RisingAndFalling);
}

void HcSr04::interruptCallback(InterruptController::Index index)
{
    if (mState == WaitForEchoStart)
    {
        mEchoStart = System::instance()->ns();
        mState = WaitForEchoEnd;
    }
    else if (mState == WaitForEchoEnd)
    {
        mState = Init;
        mDistance[mDistanceIndex++] = (System::instance()->ns() - mEchoStart) / 100 / 58;
        if (mDistanceIndex > DISTANCE_COUNT) mDistanceIndex = 0;
        System::instance()->postEvent(&mEvent);
    }
}

void HcSr04::eventCallback(System::Event* event)
{
    if (event == &mEvent)
    {
        uint32_t min = -1, max = 0, sum = 0;
        int minAt = 0, maxAt = 0;
        for (int i = 0; i < DISTANCE_COUNT; ++i)
        {
            sum += mDistance[i];
            if (mDistance[i] < min)
            {
                min = mDistance[i];
                minAt = i;
            }
            if (mDistance[i] > max)
            {
                max = mDistance[i];
                maxAt = i;
            }
        }
        sum -= mDistance[minAt] + mDistance[maxAt];
        sum /= DISTANCE_COUNT - 2;
        mAvgDistance = sum;
    }
}
