#ifndef EYES_H
#define EYES_H

#include "../SysTickControl.h"
#include "../hw/ssd1306.h"

class Eyes : public System::Event::Callback
{
public:
    enum State { Open, Close, Blink, LookLeft, LookRight };
    Eyes(SysTickControl &sysTick, Ssd1306 *left, Ssd1306 *right);

    void start(State left, State right);
    void back();

private:
    static const uint8_t* OPEN_SEQ[];
    static const uint8_t* CLOSE_SEQ[];
    static const uint8_t* BLINK_SEQ[];
    static const uint8_t* LOOK_LEFT_SEQ[];
    static const uint8_t* LOOK_RIGHT_SEQ[];


    Ssd1306* mLeft;
    Ssd1306* mRight;
    SysTickControl::RepeatingEvent mTimer;

    const uint8_t** mLeftSeq = 0;
    int mLeftSeqIndex = 0;
    int mLeftSeqLen = 0;
    const uint8_t** mRightSeq = 0;
    int mRightSeqIndex = 0;
    int mRightSeqLen = 0;
    bool mBack;

    void eventCallback(System::Event *event);
};

#endif // EYES_H
