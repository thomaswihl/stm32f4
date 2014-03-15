#ifndef CARCONTROLLER_H
#define CARCONTROLLER_H

#include "../System.h"
#include "../SysTickControl.h"
#include "../hw/hcsr04.h"
#include "../Commands.h"

class CarController : public System::Event::Callback
{
public:
    CarController(SysTickControl &sysTick, HcSr04& distanceMeasure1, HcSr04& distanceMeasure2, CmdMotor& motor, unsigned steering, unsigned propulsion);

    void eventCallback(System::Event *event);

protected:
    bool steer(int left);
    bool speed(int forward);

private:
    unsigned mCounter;
    SysTickControl::RepeatingEvent mTimer;
    HcSr04& mDistanceLeft;
    HcSr04& mDistanceRight;
    CmdMotor& mMotor;
    unsigned mSteering;
    unsigned mPropulsion;

};

#endif // CARCONTROLLER_H
