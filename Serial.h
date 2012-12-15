/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "System.h"
#include "ClockControl.h"
#include "InterruptController.h"
#include "Dma.h"
#include "CircularBuffer.h"

#include <queue>

class Serial : public InterruptController::Callback, public ClockControl::Callback, public Dma::Stream::Callback
{
public:
    enum class WordLength { Eight, Nine };
    enum class Parity { None = 0, Even = 2, Odd = 3 };
    enum class StopBits { One, Half, Two, OneAndHalf };
    enum class HardwareFlowControl { None, Cts, Rts, CtsRts };

    enum class EventType : System::Event::Type { ReceivedByte };

    Serial(System& system, System::BaseAddress base, System::Event::Component component, ClockControl* clockControl, ClockControl::Clock clock);
    virtual ~Serial();

    void setSpeed(uint32_t speed);
    void setWordLength(WordLength dataBits);
    void setParity(Parity parity);
    void setStopBits(StopBits stopBits);
    void setHardwareFlowControl(HardwareFlowControl hardwareFlow);
    void enable(bool enable);

    void config(uint32_t speed, WordLength dataBits = WordLength::Eight, Parity parity = Parity::None, StopBits stopBits = StopBits::One, HardwareFlowControl hardwareFlow = HardwareFlowControl::None);
    void configDma(Dma::Stream* tx, Dma::Stream* rx);
    void configInterrupt(InterruptController::Line* interrupt);

    int read(char* data, int size);
    unsigned int write(const char* data, unsigned int size);

    bool pop(char& c);
    bool push(char c);

protected:
    virtual void interruptCallback(InterruptController::Index index);
    virtual void clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock);
    virtual void dmaCallback(Dma::Stream::Callback::Reason reason);

private:
    struct USART
    {
        struct __SR
        {
            uint32_t PE : 1;
            uint32_t FE : 1;
            uint32_t NF : 1;
            uint32_t ORE : 1;
            uint32_t IDLE : 1;
            uint32_t RXNE : 1;
            uint32_t TC : 1;
            uint32_t TXE : 1;
            uint32_t LBD : 1;
            uint32_t CTS : 1;
            uint32_t __RESERVED0 : 22;
        }   SR;
        uint32_t DR;
        struct __BRR
        {
            uint32_t DIV_FRACTION : 4;
            uint32_t DIV_MANTISSA : 12;
            uint32_t __RESERVED0 : 16;
        }   BRR;
        struct __CR1
        {
            uint32_t SBK : 1;
            uint32_t RWU : 1;
            uint32_t RE : 1;
            uint32_t TE : 1;
            uint32_t IDLEIE : 1;
            uint32_t RXNEIE : 1;
            uint32_t TCIE : 1;
            uint32_t TXEIE : 1;
            uint32_t PEIE : 1;
            uint32_t PS : 1;
            uint32_t PCE : 1;
            uint32_t WAKE : 1;
            uint32_t M : 1;
            uint32_t UE : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t OVER8 : 1;
            uint32_t __RESERVED1 : 16;
        }   CR1;
        struct __CR2
        {
            uint32_t ADD : 4;
            uint32_t __RESERVED0 : 1;
            uint32_t LBDL : 1;
            uint32_t LBDIE : 1;
            uint32_t __RESERVED1 : 1;
            uint32_t LBCL : 1;
            uint32_t CPHA : 1;
            uint32_t CPOL : 1;
            uint32_t CLKEN : 1;
            uint32_t STOP : 2;
            uint32_t LINEN : 1;
            uint32_t __RESERVED2 : 17;
        }   CR2;
        struct __CR3
        {
            uint32_t EIE : 1;
            uint32_t IREN : 1;
            uint32_t IRLP : 1;
            uint32_t HDSEL : 1;
            uint32_t NACK : 1;
            uint32_t SCEN : 1;
            uint32_t DMAR : 1;
            uint32_t DMAT : 1;
            uint32_t RTSE : 1;
            uint32_t CTSE : 1;
            uint32_t CTSIE : 1;
            uint32_t ONEBIT : 1;
            uint32_t __RESERVED0 : 20;
        }   CR3;
        struct __GTPR
        {
            uint32_t PSC : 8;
            uint32_t GT : 8;
            uint32_t __RESERVED0 : 16;
        }   GTPR;
    };

    enum { READ_BUFFER_SIZE = 256, WRITE_BUFFER_SIZE = 256 };

    System& mSystem;
    volatile USART* mBase;
    System::Event::Component mComponent;
    ClockControl* mClockControl;
    ClockControl::Clock mClock;
    uint32_t mSpeed;
    InterruptController::Line* mInterrupt;
    Dma::Stream* mDmaTx;
    Dma::Stream* mDmaRx;
    CircularBuffer<char> mReadBuffer;
    CircularBuffer<char> mWriteBuffer;
    unsigned int mDmaTransferLength;

    void triggerWrite();

};

#endif // SERIAL_H
