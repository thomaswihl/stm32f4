#include "eyes.h"
#include "images.h"

#include <array>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

const uint8_t* Eyes::OPEN_SEQ[] = { img_0001 };
const uint8_t* Eyes::CLOSE_SEQ[] = { img_0001, img_0002, img_0003, img_0004, img_0005 };
const uint8_t* Eyes::BLINK_SEQ[] = { img_0001, img_0002, img_0003, img_0004, img_0005, img_0004, img_0003, img_0002, img_0001 };
const uint8_t* Eyes::LOOK_LEFT_SEQ[] = { img_0006, img_0007, img_0008, img_0009, img_0010 };
const uint8_t* Eyes::LOOK_RIGHT_SEQ[] = { img_0011, img_0012, img_0013, img_0014, img_0015 };

Eyes::Eyes(SysTickControl& sysTick, Ssd1306* left, Ssd1306* right) :
    mLeft(left),
    mRight(right),
    mTimer(*this, 40)
{
    sysTick.addRepeatingEvent(&mTimer);
}

void Eyes::start(Eyes::State left, Eyes::State right)
{
    struct SequenceDescription
    {
        const uint8_t** sequence;
        int length;
    };

    const std::array<SequenceDescription, 5> sequences{ {
        { OPEN_SEQ, ARRAY_SIZE(OPEN_SEQ) },
        { CLOSE_SEQ, ARRAY_SIZE(CLOSE_SEQ) },
        { BLINK_SEQ, ARRAY_SIZE(BLINK_SEQ) },
        { LOOK_LEFT_SEQ, ARRAY_SIZE(LOOK_LEFT_SEQ) },
        { LOOK_RIGHT_SEQ, ARRAY_SIZE(LOOK_RIGHT_SEQ) }
    } };
    mBack = false;
    mLeftSeq = sequences[left].sequence;
    mLeftSeqLen = sequences[left].length;
    mLeftSeqIndex = 0;
    mRightSeq = sequences[right].sequence;
    mRightSeqLen = sequences[right].length;
    mRightSeqIndex = 0;
}

void Eyes::back()
{
    mBack = true;
}

void Eyes::eventCallback(System::Event *event)
{
    if (mBack)
    {
        if (mLeft != nullptr && mLeftSeqIndex > 0)
        {
            mLeft->sendData(mLeftSeq[--mLeftSeqIndex]);
        }
        if (mRight != nullptr && mRightSeqIndex > 0)
        {
            mRight->sendData(mRightSeq[--mRightSeqIndex]);
        }
    }
    else
    {
        if (mLeft != nullptr && mLeftSeqIndex < mLeftSeqLen)
        {
            mLeft->sendData(mLeftSeq[mLeftSeqIndex++]);
        }
        if (mRight != nullptr && mRightSeqIndex < mRightSeqLen)
        {
            mRight->sendData(mRightSeq[mRightSeqIndex++]);
        }
    }
}
