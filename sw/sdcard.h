#ifndef SDCARD_H
#define SDCARD_H

#include "../sdio.h"

class SdCard : public System::Event::Callback
{
public:
    // supplyVoltage is in 1/10 Volt, i.e. 30 for 3V, 33 for 3.3V, ...
    SdCard(Sdio& sdio, int supplyVoltage);

    void init();

private:
    enum class StateResult { Repeat, Continue, Stop };
    struct Command
    {
        bool active;
        uint8_t cmd;
        uint32_t arg;
        Sdio::Response response;
    };
    typedef StateResult(SdCard::*StateFunc)();
    struct StateData
    {
        uint32_t step;
        System::Event::Result lastResult;
        union
        {
            struct
            {
                uint32_t expectedResult;
            }   interfaceCondition;
        }   privateData;
    };

    static const unsigned CLOCK_IDENTIFICATION = 400000;
    static const uint8_t CHECK_PATTERN = 0xaa;
    static const StateFunc mInit[];

    System::Event mEvent;
    Sdio& mSdio;
    int mVolt;
    int mDebugLevel;
    const StateFunc* mStateFunc;
    unsigned mStateFuncCount;
    unsigned mStateFuncActive;
    StateData mStateData;
    Command mCmd;
    Command mAppCmd;


    struct
    {
        unsigned mRca;
        unsigned mNAC;
        bool mHcSupport;
        bool mHc;
        int mDCtrlBlockSize;
        unsigned mTaac;
        unsigned mNsac;
        unsigned mTransferRate;
        unsigned mCommandClass;
        unsigned mBlockCount;
        unsigned mReadBlockSize;
        bool mPartialBlockRead;
        bool mReadBlockMisalign;
        unsigned mWriteBlockSize;
        bool mPartialBlockWrite;
        bool mWriteBlockMisalign;
        bool mDriverStageImplemented;
        unsigned mReadCurrentMin;
        unsigned mReadCurrentMax;
        unsigned mWriteCurrentMin;
        unsigned mWriteCurrentMax;
        bool mEraseSingleBlock;
        unsigned mEraseBlockSize;
        unsigned mWriteProtectGroupSize;
        bool mWriteProtectGroupEnabled;
        unsigned mReadToWriteFactor;
        bool mCopy;
        bool mPermanentlyWriteProtected;
        bool mTemporarilyWriteProtected;
        bool mFileFormatGroup;
        unsigned mFileFormat;
        unsigned mBusWidth;
        bool mDataStatusAfterErase;
        unsigned mSpecVersion;
        bool mSetBlockCountSupport;
        bool mSpeedClassControlSupport;
    }   mCardInfo;

    StateResult reset();                    // CMD0
    StateResult interfaceCondition();       // CMD8
    StateResult initializeCard();           // ACMD41
    StateResult getCardIdentifier();        // CMD2
    bool getRelativeCardAddress();        // CMD3
    bool getCardSpecificData();           // CMD9
    bool selectCard(bool select = true);  // CMD7
    bool getCardStatus();                 // CMD13
    bool getCardConfiguration();          // ACMD51
    bool setBusWidth();                   // ACMD6
    bool setBlockSize(uint16_t blockSize);

    virtual void eventCallback(System::Event* event);
    void executeSteps(const StateFunc* functions, unsigned count);
    void executeStep();

    void sendCommand(uint8_t cmd, uint32_t arg, Sdio::Response response);
    void sendAppCommand(uint8_t cmd, uint32_t arg, Sdio::Response response);

    bool checkCardStatus(uint32_t status);
    uint32_t ocrFromVoltage(int volt);
    void voltageFromOcr(uint32_t ocr, int &minVoltage, int &maxVoltage);
    void printOcr();

    uint32_t getBits(uint8_t *field, int fieldSize, int highestBit, int lowestBit);
    const char* const toResult(System::Event::Result result);
};

#endif // SDCARD_H
