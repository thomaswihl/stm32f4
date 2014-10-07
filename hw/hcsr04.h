#ifndef HCSR04_H
#define HCSR04_H

#include "../System.h"
#include "../Gpio.h"
#include "../SysTickControl.h"
#include <stdint.h>

class HcSr04 : public InterruptController::Callback, public System::Event::Callback
{
public:
    HcSr04(SysTickControl& sysTick, Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq);
    void addDevice(Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq);
    void start(unsigned index);
    uint32_t distance(int index) { return mDistance[index]; } // in mm
    void clear();

private:
    void interruptCallback(InterruptController::Index index);
    void eventCallback(System::Event *event);

private:
    enum State { Init, SendStartPulse, WaitForEchoStart, WaitForEchoEnd };
    std::vector<Gpio::ConfigurablePin> mPins;
    std::vector<ExternalInterrupt::Line*> mIrqs;
    System::Event mEvent;
    State mState;
    unsigned mIndex;
    std::vector<uint64_t> mEchoStart;
    std::vector<uint32_t> mDistance;
};

#endif // HCSR04_H
