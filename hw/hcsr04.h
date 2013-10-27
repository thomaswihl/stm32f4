#ifndef HCSR04_H
#define HCSR04_H

#include "../System.h"
#include "../Gpio.h"
#include <stdint.h>

class HcSr04 : public InterruptController::Callback
{
public:
    HcSr04(Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq);
    void start();
    uint32_t distance() { return mDistance; } // in mm

private:
    void interruptCallback(InterruptController::Index index);

private:
    enum State { Init, SendStartPulse, WaitForEchoStart, WaitForEchoEnd };
    Gpio::ConfigurablePin& mPin;
    ExternalInterrupt::Line* mIrq;
    State mState;
    uint64_t mEchoStart;
    uint32_t mDistance;
};

#endif // HCSR04_H
