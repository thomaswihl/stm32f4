#include "carcontroller.h"

CarController::CarController(SysTickControl &sysTick, HcSr04& distanceRight, HcSr04& distanceLeft, CmdMotor &motor, unsigned steering, unsigned propulsion) :
    mCounter(0),
    mTimer(*this, 100),
    mDistanceLeft(distanceLeft),
    mDistanceRight(distanceRight),
    mMotor(motor),
    mSteering(steering),
    mPropulsion(propulsion)
{
    sysTick.addRepeatingEvent(&mTimer);
}


void CarController::eventCallback(System::Event *event)
{
    if (event == &mTimer)
    {
        if ((mCounter % 2) == 0)
        {
            mDistanceLeft.start();
        }
        else
        {
            unsigned minDistance = std::min(mDistanceLeft.distance(), mDistanceRight.distance());
            if (minDistance > 250)
            {
                steer(100 * static_cast<int>(mDistanceRight.distance() - mDistanceLeft.distance()) / static_cast<int>(mDistanceLeft.distance() + mDistanceRight.distance()));
            }
            else
            {
                steer(0);
            }
            if (minDistance > 500) speed(100);
            else if (minDistance > 250) speed(50);
            else speed(0);
            mDistanceRight.start();
        }
        ++mCounter;
    }
}

bool CarController::steer(int left)
{
    return mMotor.set(mSteering, left);
}

bool CarController::speed(int forward)
{
    return mMotor.set(mPropulsion, forward);
}
