#include "hcsr04.h"

HcSr04::HcSr04(SysTickControl &sysTick, Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq) :
    mPins(&pin, &pin + 1), mIrqs(&irq, &irq + 1), mEvent(*this), mState(Init)
{
    pin.configOutput(Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Medium);
    clear();
}

void HcSr04::addDevice(Gpio::ConfigurablePin &pin, ExternalInterrupt::Line *irq)
{
    mPins.push_back(pin);
    mIrqs.push_back(irq);
    pin.configOutput(Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Medium);
    mEchoStart.resize(mPins.size());
    mDistance.resize(mPins.size());
}

void HcSr04::start()
{
    //printf("State = %i\n", mState);
    mState = SendStartPulse;
    for (auto& irq : mIrqs) irq->disable();
    for (auto& pin : mPins)
    {
        pin.setMode(Gpio::Mode::Output);
        pin.set();
    }
    System::instance()->usleep(20);
    for (auto& pin : mPins)
    {
        pin.setMode(Gpio::Mode::Input);
    }
    mState = WaitForEchoStart;
    for (auto& irq : mIrqs)
    {
        irq->setCallback(this);
        irq->enable(ExternalInterrupt::Trigger::RisingAndFalling);
    }
}

void HcSr04::clear()
{
    for (unsigned i = 0; i < mDistance.size(); ++i) mDistance[i] = 0;
}

void HcSr04::interruptCallback(InterruptController::Index index)
{
    unsigned i = -1;
    for (i = 0; i < mIrqs.size(); ++i)
    {
        if (mIrqs[i]->index() == index) break;
    }
    if (i >= mIrqs.size()) return;
    if (mPins[i].get()) mEchoStart[i] = System::instance()->ns();
    else
    {
        mDistance[i] = (System::instance()->ns() - mEchoStart[i]) / 100 / 58;
    }
}

void HcSr04::eventCallback(System::Event* event)
{
}
