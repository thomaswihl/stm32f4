#ifndef CARCONTROLLER_H
#define CARCONTROLLER_H

#include "../System.h"
#include "../SysTickControl.h"
#include "../hw/hcsr04.h"
#include "../hw/tlc5940.h"
#include "../Commands.h"
#include "eyes.h"



class CarController : public System::Event::Callback
{
public:
    CarController(SysTickControl &sysTick, HcSr04& distance, int distanceLeft, int distanceRight, int distanceFront,
                  CmdMotor& motor, unsigned steering, unsigned propulsion, unsigned measureDirection,
                  Tlc5940& light);

    void setEyes(Eyes* eyes) { mEyes = eyes; }
    void eventCallback(System::Event *event);
    void start();
    void stop();
    bool running() { return mRunning; }
    static void init(StmSystem& sys, CommandInterpreter &interpreter);

protected:
    bool steer(int left);
    bool setSpeed(int forward);
    bool look(int left);
    void newPosition(int position, int which);
    inline uint32_t position(int which, int time = HISTORY_SIZE) { return mParam[which][POSITION][time]; }
    inline uint32_t speed(int which, int time = HISTORY_SIZE) { return mParam[which][SPEED][time]; }
    inline uint32_t acceleration(int which, int time = HISTORY_SIZE) { return mParam[which][ACCELERATION][time]; }
    void setBlink(bool left, bool on = true);

private:
    static const unsigned RIGHT_REVERSE_LIGHT = 0;
    static const unsigned RIGHT_BACK_LIGHT = 1;
    static const unsigned RIGHT_BREAK_LIGHT = 2;
    static const unsigned RIGHT_BLINK_BACK_LIGHT = 3;

    static const unsigned LEFT_REVERSE_LIGHT = 4;
    static const unsigned LEFT_BACK_LIGHT = 5;
    static const unsigned LEFT_BREAK_LIGHT = 6;
    static const unsigned LEFT_BLINK_BACK_LIGHT = 7;

    static const unsigned RIGHT_SIDE_TOP_LIGHT = 8;
    static const unsigned RIGHT_SIDE_BOTTOM_LIGHT = 9;
    static const unsigned RIGHT_FRONT_RIGHT_LIGHT = 10;
    static const unsigned RIGHT_FRONT_LEFT_LIGHT = 11;

    static const unsigned LEFT_SIDE_TOP_LIGHT = 12;
    static const unsigned LEFT_SIDE_BOTTOM_LIGHT = 13;
    static const unsigned LEFT_FRONT_RIGHT_LIGHT = 14;
    static const unsigned LEFT_FRONT_LEFT_LIGHT = 15;

    enum State { Stop, Forward, SteerLeft, SteerRight, ReverseLeft, ReverseRight };

    State mState;
    int mCounter;
    SysTickControl::RepeatingEvent mTimer;
    HcSr04& mDistance;
    int mDistanceLeft;
    int mDistanceRight;
    int mDistanceFront;
    CmdMotor& mMotor;
    unsigned mSteering;
    unsigned mPropulsion;
    unsigned mMeasureDirection;
    Tlc5940& mLight;
    Eyes* mEyes;


    bool mRunning;
    int mSpeed;
    int mDestinationSpeed;
    int mLastDistanceIndex[2];
    static const int POSITION = 0;
    static const int SPEED = 1;
    static const int ACCELERATION = 2;
    static const int HISTORY_SIZE = 2;
    static const int LEFT = 0;
    static const int CENTER = 1;
    static const int RIGHT = 2;

    int mParam[3][3][HISTORY_SIZE + 1];
};

#endif // CARCONTROLLER_H
