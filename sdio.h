#ifndef SDIO_H
#define SDIO_H

#include "System.h"
#include "InterruptController.h"
#include "Dma.h"
#include "ClockControl.h"

class Sdio
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

    bool sendCommand(uint8_t cmd, uint32_t arg, Response response);
    uint32_t shortResponse();
    void longResponse(uint8_t* response);

    void prepareTransfer(Direction direction, uint32_t *data, unsigned byteCount);
    bool setBlockSize(uint16_t blockSize);
    void setDataTimeout(uint32_t clocks);

    void setBusWidth(BusWidth width);
private:

    static const unsigned PLL_CLOCK = 48000000;
    static const unsigned IC_MASK = 0x00c007ff;
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

    volatile SDIO* mBase;
    InterruptController::Line& mIrq;
    Dma::Stream& mDma;
    int mDebugLevel;

    const char* toString(Response response);
};

#endif // SDIO_H
