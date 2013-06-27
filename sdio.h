#ifndef SDIO_H
#define SDIO_H

#include "System.h"
#include "InterruptController.h"
#include "Dma.h"
#include "ClockControl.h"

class Sdio : public Dma::Stream::Callback, public InterruptController::Callback
{
public:
    enum class Response { None, Short, Long, ShortNoCrc, LongNoCrc };
    enum class Direction { Read, Write };
    enum class BusWidth { OneDataLine, FourDataLines, EightDataLines };
    Sdio(System::BaseAddress base, InterruptController::Line& irq, Dma::Stream& dma);

    void enable(bool enable);
    void setClock(unsigned speed);
    uint32_t clock();

    void reset();
    void printHostStatus();

    bool sendCommand(uint8_t cmd, uint32_t arg, Response response, System::Event& completeEvent);
    uint32_t shortResponse();
    void longResponse(uint8_t* response);

    void prepareTransfer(Direction direction, uint32_t *data, unsigned byteCount);
    bool setBlockSize(uint16_t blockSize);
    void setDataTimeout(uint32_t clocks);

    void setBusWidth(BusWidth width);
private:

    static const unsigned PLL_CLOCK = 48000000;

    static const uint32_t CCRCFAIL  = 0x00000001;
    static const uint32_t DCRCFAIL  = 0x00000002;
    static const uint32_t CTIMEOUT  = 0x00000004;
    static const uint32_t DTIMEOUT  = 0x00000008;
    static const uint32_t TXUNDERR  = 0x00000010;
    static const uint32_t RXOVERR   = 0x00000020;
    static const uint32_t CMDREND   = 0x00000040;
    static const uint32_t CMDSENT   = 0x00000080;
    static const uint32_t DATAEND   = 0x00000100;
    static const uint32_t STBITERR  = 0x00000200;
    static const uint32_t DBCKEND   = 0x00000400;
    static const uint32_t CMDACT    = 0x00000800;
    static const uint32_t TXACT     = 0x00001000;
    static const uint32_t RXACT     = 0x00002000;
    static const uint32_t TXFIFOHE  = 0x00004000;
    static const uint32_t RXFIFOHF  = 0x00008000;
    static const uint32_t TXFIFOOF  = 0x00010000;
    static const uint32_t RXFIFOOF  = 0x00020000;
    static const uint32_t TXFIFOOE  = 0x00040000;
    static const uint32_t RXFIFOOE  = 0x00080000;
    static const uint32_t TXDVAL    = 0x00100000;
    static const uint32_t RXDVAL    = 0x00200000;
    static const uint32_t SDIOIT    = 0x00400000;
    static const uint32_t CEATAEND  = 0x00800000;

    static const unsigned IC_MASK = 0x00c007ff;
    static const unsigned IM_MASK = CCRCFAIL | DCRCFAIL | CTIMEOUT | DTIMEOUT | TXUNDERR | RXOVERR | CMDREND | CMDSENT | DATAEND | STBITERR;

    static const char* const STATUS_MSG[];

    enum class State { Idle, Ready, Ident, Standby, Transfer, Data, Receive, Program, Disabled };

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
        union __DCTRL
        {
            struct
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
            }   bits;
            uint32_t value;
        }   DCTRL;
        uint32_t DCOUNT;
        uint32_t STA;
        uint32_t ICR;
        uint32_t MASK;
        uint32_t __RESERVED0[2];
        uint32_t FIFOCNT;
        uint32_t __RESERVED1[13];
        uint32_t FIFO[32];
    };

    volatile SDIO* mBase;
    InterruptController::Line& mIrq;
    Dma::Stream& mDma;
    int mDebugLevel;
    bool mIgnoreCrc;
    uint8_t mLastCommand;
    System::Event* mCompleteEvent;


    virtual void dmaCallback(Dma::Stream* stream, Dma::Stream::Callback::Reason reason);
    virtual void interruptCallback(InterruptController::Index index);

    const char* toString(Response response);
};

#endif // SDIO_H
