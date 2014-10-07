#include "hcsr04.h"

HcSr04::HcSr04(SysTickControl &sysTick, Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq) :
    mPins(&pin, &pin + 1), mIrqs(&irq, &irq + 1), mEvent(*this), mState(Init), mIndex(-1)
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

void HcSr04::start(unsigned index)
{
    if (index >= mPins.size()) return;
    mIndex = index;
    mState = SendStartPulse;
    mIrqs[index]->disable();
    mPins[index].setMode(Gpio::Mode::Output);
    mPins[index].set();
    System::instance()->usleep(20);
    mPins[index].setMode(Gpio::Mode::Input);
    mState = WaitForEchoStart;
    mIrqs[index]->setCallback(this);
    mIrqs[index]->enable(ExternalInterrupt::Trigger::RisingAndFalling);
}

void HcSr04::clear()
{
    for (unsigned i = 0; i < mDistance.size(); ++i) mDistance[i] = 0;
}

void HcSr04::interruptCallback(InterruptController::Index /*index*/)
{
    if (mIndex >= mPins.size()) return;
    if (mPins[mIndex].get()) mEchoStart[mIndex] = System::instance()->ns();
    else
    {
        mDistance[mIndex] = (System::instance()->ns() - mEchoStart[mIndex]) / 100 / 58;
        mIrqs[mIndex]->disable();
        //System::instance()->postEvent(&mEvent);
    }
}

void HcSr04::eventCallback(System::Event* event)
{
    if (event == &mEvent)
    {
        if (mIndex < mPins.size() - 1)
        {
            start(mIndex + 1);
        }
        else
        {
            mIndex = -1;
        }
    }
}
