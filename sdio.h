#ifndef SDIO_H
#define SDIO_H

#include "System.h"
#include "ClockControl.h"

class Sdio
{
public:
    Sdio(System::BaseAddress base, ClockControl* clockControl);

private:
    struct SDIO
    {
        struct __POWER
        {
            uint32_t PWRCTRL : 2;
            uint32_t __RESERVED0 : 30;
        }   POWER;
        struct __CLKCR
        {
            uint32_t CLKDIV : 8;
            uint32_t CLKEN : 1;
            uint32_t PWRSAV : 1;
            uint32_t BYPASS : 1;
            uint32_t WIDBUS : 2;
            uint32_t NEGEDGE : 1;
            uint32_t HWFC_EN : 1;
            uint32_t __RESERVED0 : 17;
        }   CLKCR;
        uint32_t ARG;
        struct __CMD
        {
            uint32_t CMDINDEX : 6;
            uint32_t WAITRESP : 2;
            uint32_t WAITINT : 1;
            uint32_t WAITPEND : 1;
            uint32_t CPSMEN : 1;
            uint32_t SDIO_SUSPEND : 1;
            uint32_t EN_CMD_COMPL : 1;
            uint32_t NIEN : 1;
            uint32_t ATACMD : 1;
            uint32_t __RESERVED0 : 17;
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
        struct __STA
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
        }   STA;
        struct __ICR
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
        }   ICR;
        struct __MASK
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
        }   MASK;
        uint32_t __RESERVED0[2];
        uint32_t FIFOCNT;
        uint32_t __RESERVED1[13];
        uint32_t FIFO;
    };
};

#endif // SDIO_H
