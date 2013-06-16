#ifndef SDIO_H
#define SDIO_H

#include "System.h"
#include "ClockControl.h"

class Sdio
{
public:
    enum class Result { Ok, Timeout, CrcError, Error };
    // supplyVoltage is in 1/10 Volt, i.e. 30 for 3V, 33 for 3.3V, ...
    Sdio(System::BaseAddress base, int supplyVoltage);

    void enable(bool enable);
    void setClock(unsigned clock);
    uint32_t clock();

    void printHostStatus();
    void resetCard();
    bool initCard();
    Result interfaceCondition();            // CMD8
    Result initializeCard(bool hcSupport);  // ACMD41
    Result getCardIdentifier();             // CMD2
    Result getRelativeCardAddress();        // CMD3
    Result getCardSpecificData();           // CMD9
    Result selectCard(bool select = true);  // CMD7
    Result getCardStatus();                 // CMD13
    Result getCardConfiguration();          // ACMD51

private:

    static const unsigned CLOCK_IDENTIFICATION = 400000;
    static const uint8_t CHECK_PATTERN = 0xaa;
    static const unsigned PLL_CLOCK = 48000000;
    static const char* const STATUS_MSG[];

    enum class State { Idle, Ready, Ident, Standby, Transfer, Data, Receive, Program, Disabled };
    enum class Response { None, Short, Long, ShortNoCrc, LongNoCrc };

    struct SDIO
    {
        struct __POWER
        {
            uint32_t PWRCTRL : 2;
            uint32_t __RESERVED0 : 30;
        }   POWER;
        union __CLKCR
        {
            struct
            {
                uint32_t CLKDIV : 8;
                uint32_t CLKEN : 1;
                uint32_t PWRSAV : 1;
                uint32_t BYPASS : 1;
                uint32_t WIDBUS : 2;
                uint32_t NEGEDGE : 1;
                uint32_t HWFC_EN : 1;
                uint32_t __RESERVED0 : 17;
            }   bits;
            uint32_t value;
        }   CLKCR;
        uint32_t ARG;
        union __CMD
        {
            struct
            {
                uint32_t CMD_WAIT : 8;
                uint32_t WAITINT : 1;
                uint32_t WAITPEND : 1;
                uint32_t CPSMEN : 1;
                uint32_t SDIO_SUSPEND : 1;
                uint32_t EN_CMD_COMPL : 1;
                uint32_t NIEN : 1;
                uint32_t ATACMD : 1;
                uint32_t __RESERVED0 : 17;
            }   bits;
            uint32_t value;
        }   CMD;
        uint32_t RESPCMD;
        uint32_t RESP[4];
        uint32_t DTIMER;
        uint32_t DLEN;
        struct __DCTRL
        {
            uint32_t DTEN : 1;
            uint32_t DTDIR : 1;
            uint32_t DTMODE : 1;
            uint32_t DMAEN : 1;
            uint32_t DBLOCKSIZE : 4;
            uint32_t RWSTART : 1;
            uint32_t RWSTOP : 1;
            uint32_t RWMOD : 1;
            uint32_t SDIOEN : 1;
            uint32_t __RESERVED0 : 20;
        }   DCTRL;
        uint32_t DCOUNT;
        union __STA
        {
            struct
            {
                uint32_t CCRCFAIL : 1;
                uint32_t DCRCFAIL : 1;
                uint32_t CTIMEOUT : 1;
                uint32_t DTIMEOUT : 1;
                uint32_t TXUNDERR : 1;
                uint32_t RXOVERR : 1;
                uint32_t CMDREND : 1;
                uint32_t CMDSENT : 1;
                uint32_t DATAEND : 1;
                uint32_t STBITERR : 1;
                uint32_t DBCKEND : 1;
                uint32_t CMDACT : 1;
                uint32_t TXACT : 1;
                uint32_t RXACT : 1;
                uint32_t TXFIFOHE : 1;
                uint32_t RXFIFOHF : 1;
                uint32_t TXFIFOOF : 1;
                uint32_t RXFIFOOF : 1;
                uint32_t TXFIFOOE : 1;
                uint32_t RXFIFOOE : 1;
                uint32_t TXDVAL : 1;
                uint32_t RXDVAL : 1;
                uint32_t SDIOIT : 1;
                uint32_t CEATAEND : 1;
                uint32_t __RESERVED0 : 8;
            }   bits;
            uint32_t value;
        }   STA;
        union __ICR
        {
            struct
            {
                uint32_t CCRCFAILC : 1;
                uint32_t DCRCFAILC : 1;
                uint32_t CTIMEOUTC : 1;
                uint32_t DTIMEOUTC : 1;
                uint32_t TXUNDERRC : 1;
                uint32_t RXOVERRC : 1;
                uint32_t CMDRENDC : 1;
                uint32_t CMDSENTC : 1;
                uint32_t DATAENDC : 1;
                uint32_t STBITERRC : 1;
                uint32_t DBCKENDC : 1;
                uint32_t __RESERVED : 11;
                uint32_t SDIOITC : 1;
                uint32_t CEATAENDC : 1;
                uint32_t __RESERVED0 : 8;
            }   bits;
            uint32_t value;
        }   ICR;
        union __MASK
        {
            struct
            {
                uint32_t CCRCFAILIE : 1;
                uint32_t DCRCFAILIE : 1;
                uint32_t CTIMEOUTIE : 1;
                uint32_t DTIMEOUTIE : 1;
                uint32_t TXUNDERRIE : 1;
                uint32_t RXOVERRIE : 1;
                uint32_t CMDRENDIE : 1;
                uint32_t CMDSENTIE : 1;
                uint32_t DATAENDIE : 1;
                uint32_t STBITERRIE : 1;
                uint32_t DBCKENDIE : 1;
                uint32_t CMDACTIE : 1;
                uint32_t TXACTIE : 1;
                uint32_t RXACTIE : 1;
                uint32_t TXFIFOHEIE : 1;
                uint32_t RXFIFOHFIE : 1;
                uint32_t TXFIFOOFIE : 1;
                uint32_t RXFIFOOFIE : 1;
                uint32_t TXFIFOOEIE : 1;
                uint32_t RXFIFOOEIE : 1;
                uint32_t TXDVALIE : 1;
                uint32_t RXDVALIE : 1;
                uint32_t SDIOITIE : 1;
                uint32_t CEATAENDIE : 1;
                uint32_t __RESERVED0 : 8;
            }   bits;
            uint32_t value;
        }   MASK;
        uint32_t __RESERVED0[2];
        uint32_t FIFOCNT;
        uint32_t __RESERVED1[13];
        uint32_t FIFO[32];
    };

    union CID
    {
        struct
        {
            uint8_t MID;
            uint8_t OID[2];
            uint8_t PNM[5];
            uint8_t PRV;
            uint8_t PSN[4];
            uint8_t MDT[2];
            uint8_t CRC;
        }   bits;
        uint32_t value[4];
    };

    volatile SDIO* mBase;
    int mVolt;
    bool mDebug;
    CID mCid;
    unsigned mRca;
    struct
    {
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
    }   mCsd;

    Result sendCommand(uint8_t cmd, uint32_t arg, Response response);
    Result sendAppCommand(uint8_t cmd, uint32_t arg, Response response);
    bool checkCardStatus(uint32_t status);
    uint32_t ocrFromVoltage(int volt);
    void voltageFromOcr(uint32_t ocr, int& minVoltage, int& maxVoltage);
    void printOcr();
    const char* toString(Response response);
    uint32_t getBits(uint8_t* field, int fieldSize, int highestBit, int lowestBit);
};

#endif // SDIO_H
