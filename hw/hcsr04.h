#ifndef HCSR04_H
#define HCSR04_H

#include "../System.h"
#include "../Gpio.h"
#include <stdint.h>

class HcSr04 : public InterruptController::Callback, public System::Event::Callback
{
public:
    HcSr04(Gpio::ConfigurablePin& pin, ExternalInterrupt::Line* irq);
    void start();
    uint32_t distance() { return mAvgDistance; } // in mm

private:
    void interruptCallback(InterruptController::Index index);
    void eventCallback(System::Event *event);

private:
    static const int DISTANCE_COUNT = 5;
    enum State { Init, SendStartPulse, WaitForEchoStart, WaitForEchoEnd };
    Gpio::ConfigurablePin& mPin;
    ExternalInterrupt::Line* mIrq;
    System::Event mEvent;
    State mState;
    uint64_t mEchoStart;
    int mDistanceIndex;
    uint32_t mDistance[DISTANCE_COUNT];
    uint32_t mAvgDistance;
};

#endif // HCSR04_H
