#include "hcsr04.h"

HcSr04::HcSr04(Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq) : mPin(pin), mIrq(irq), mState(Init)
{
    mPin.configOutput(Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Medium);
    mIrq->setCallback(this);
}

void HcSr04::start()
{
    printf("State = %i\n", mState);
    mState = SendStartPulse;
    mIrq->disable();
    mPin.setMode(Gpio::Mode::Output);
    mPin.set();
    System::instance()->usleep(20);
    mPin.setMode(Gpio::Mode::Input);
    mState = WaitForEchoStart;
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
        mDistance = (System::instance()->ns() - mEchoStart) / 100 / 58;
    }
}
