#ifndef SERIAL_H
#define SERIAL_H

#include "System.h"

class Serial : System::IrqHandler
{
public:
    enum Base : uint32_t
    {
        USART1 = 0x40011000,
        USART2 = 0x40004400,
        USART3 = 0x40004800,
        UART4 = 0x40004c00,
        UART5 = 0x40005000,
        USART6 = 0x40011400,
    };

    Serial(Base base, System::InterruptIndex irq);
    virtual ~Serial();

protected:
    virtual void irq(int index);

private:
    struct Usart
    {
        struct __SR
        {
            uint16_t PE : 1;
            uint16_t FE : 1;
            uint16_t NF : 1;
            uint16_t ORE : 1;
            uint16_t IDLE : 1;
            uint16_t RXNE : 1;
            uint16_t TC : 1;
            uint16_t TXE : 1;
            uint16_t LBD : 1;
            uint16_t CTS : 1;
            uint16_t __RESERVED0 : 6;
        }   SR;
        uint16_t __RESERVED0;
        uint16_t DR;
        uint16_t __RESERVED1;
        struct __BRR
        {
            uint16_t DIV_Fraction : 4;
            uint16_t DIV_MANTISSA : 12;
        }   BRR;
        uint16_t __RESERVED2;
        struct __CR1
        {
            uint16_t SBK : 1;
            uint16_t RWU : 1;
            uint16_t RE : 1;
            uint16_t TE : 1;
            uint16_t IDLEIE : 1;
            uint16_t RXNEIE : 1;
            uint16_t TCIE : 1;
            uint16_t TXEIE : 1;
            uint16_t PEIE : 1;
            uint16_t PS : 1;
            uint16_t PCE : 1;
            uint16_t WAKE : 1;
            uint16_t M : 1;
            uint16_t UE : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t OVER8 : 1;
        }   CR1;
        uint16_t __RESERVED3;
        struct __CR2
        {
            uint16_t ADD : 4;
            uint16_t __RESERVED0 : 1;
            uint16_t LBDL : 1;
            uint16_t LBDIE : 1;
            uint16_t __RESERVED1 : 1;
            uint16_t LBCL : 1;
            uint16_t CPHA : 1;
            uint16_t CPOL : 1;
            uint16_t CLKEN : 1;
            uint16_t STOP : 2;
            uint16_t LINEN : 1;
            uint16_t __RESERVED2 : 1;
        }   CR2;
        uint16_t __RESERVED4;
        struct __CR3
        {
            uint16_t EIE : 1;
            uint16_t IREN : 1;
            uint16_t IRLP : 1;
            uint16_t HDSEL : 1;
            uint16_t NACK : 1;
            uint16_t SCEN : 1;
            uint16_t DMAR : 1;
            uint16_t DMAT : 1;
            uint16_t RTSE : 1;
            uint16_t CTSE : 1;
            uint16_t CTSIE : 1;
            uint16_t ONEBIT : 1;
            uint16_t __RESERVED0 : 4;
        }   CR3;

        uint16_t __RESERVED5;
        struct __GTPR
        {
            uint8_t PSC;
            uint8_t GT;
        }   GTPR;
    }   __attribute__ ((__packed__));

    volatile Usart* mBase;
};

#endif // SERIAL_H
