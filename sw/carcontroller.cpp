#include "carcontroller.h"

CarController::CarController(SysTickControl &sysTick, HcSr04& distance, int distanceLeft, int distanceRight, int distanceFront,
                             CmdMotor &motor, unsigned steering, unsigned propulsion, unsigned measureDirection,
                             Tlc5940& light) :
    mState(Stop),
    mCounter(0),
    mTimer(*this, 200),
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
}


void CarController::eventCallback(System::Event *event)
{
    static const int ACCELERATION_FACTOR = 5;
    if (event == &mTimer && mRunning)
    {
        newPosition(mDistance.distance(mDistanceLeft), mDistance.distance(mDistanceFront), mDistance.distance(mDistanceRight));
        mDistance.start();
        State old = mState;
        if (mCounter < 20)
        {
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
                if (position(CENTER) < 800)
                {
                    if (position(LEFT) > position(RIGHT))
                    {
                        steer(80);
                        look(100);
                        setBlink(true, true);
                        mState = SteerLeft;
                    }
                    else
                    {
                        steer(-80);
                        look(-100);
                        setBlink(false, true);
                        mState = SteerRight;
                    }
                }
                else
                {
                    if (mSpeed < 50) mSpeed = 50;
                    else if (mSpeed < 100) mSpeed += ACCELERATION_FACTOR;
                    look((static_cast<int>(mCounter % 21) - 10) * 2);
                    int delta = position(LEFT) - position(RIGHT);
                    if (delta < 100 || delta > -100) delta = 0;
                    else if (delta >= 100) delta = 20;
                    else if (delta <= 100) delta = -20;
                    steer(delta);
                }
                break;
            case CarController::SteerLeft:
                if (position(RIGHT) > 1000)
                {
                    mState = Forward;
                }
                else if (position(LEFT) < 300 && position(CENTER) < 300 && position(RIGHT) < 300)
                {
                    mState = Stop;
                }
                break;
            case CarController::SteerRight:
                if (position(LEFT) > 1000)
                {
                    mState = Forward;
                }
                else if (position(LEFT) < 300 && position(CENTER) < 300 && position(RIGHT) < 300)
                {
                    mState = Stop;
                }
                break;
            case CarController::Reverse:
                break;

            }
        }
        setSpeed(mSpeed);
        ++mCounter;
        if (old != mState || (mCounter % 10) == 0)
        {
            printf("%i: %i,%i,%i\n", mState, position(LEFT), position(CENTER), position(RIGHT));
        }
//        if ((mCounter % 10) == 0)
//        {
//            for (int p = 0; p < 3; ++p)
//            {
//                for (int i = HISTORY_SIZE - 1; i >= 0; --i)
//                {
//                    printf("%6i ", mParam[CENTER][p][i]);
//                }
//                printf("\r\n");
//            }
//            printf("\r\n");
//        }
    }
}

bool CarController::steer(int left)
{
    return mMotor.set(mSteering, -left);
}

bool CarController::setSpeed(int forward)
{
    mSpeed = forward;
    return mMotor.set(mPropulsion, forward);
}

bool CarController::look(int left)
{
    return mMotor.set(mMeasureDirection, -left);
}

void CarController::newPosition(uint32_t left, uint32_t center, uint32_t right)
{
    for (int which = 2; which >= 0; --which)
    {
        for (int physic = 0; physic < 3; ++physic)
        {
            for (int i = HISTORY_SIZE - 1; i >= 0; --i)
            {
                mParam[which][physic][i] = mParam[which][physic][i - 1];
            }
            if (physic == 0)
            {
                if (which == LEFT) mParam[which][physic][0] = left;
                else if (which == CENTER) mParam[which][physic][0] = center;
                else if (which == RIGHT) mParam[which][physic][0] = right;
            }
            else
            {
                mParam[which][physic][0] = mParam[which][physic - 1][0] - mParam[which][physic - 1][1];
            }
        }

    }
}

void CarController::setBlink(bool left, bool on)
{
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
