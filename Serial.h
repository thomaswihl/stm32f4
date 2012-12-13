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

class Serial : public InterruptController::Handler, public ClockControl::ChangeHandler
{
public:
    enum class WordLength { Eight, Nine };
    enum class Parity { None = 0, Even = 2, Odd = 3 };
    enum class StopBits { One, Half, Two, OneAndHalf };
    enum class HardwareFlowControl { None, Cts, Rts, CtsRts };

    class Event : public System::Event
    {
    public:
        enum class Type { ReceivedByte };
        System::BaseAddress base() { return mBase; }
        Type type() { return mType; }
        Event(System::BaseAddress base, Type type) : System::Event(System::Event::Component::Serial), mBase(base), mType(type) { }

    private:
        System::BaseAddress mBase;
        Type mType;
    };

    Serial(System& system, System::BaseAddress base, ClockControl* clockControl, ClockControl::Clock clock);
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
    int write(const char* data, int size);

protected:
    virtual void handle(InterruptController::Index index);
    virtual void clockPrepareChange(uint32_t newClock);
    virtual void clockChanged(uint32_t newClock);

private:
    struct USART
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
            uint16_t DIV_FRACTION : 4;
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
            uint16_t PSC : 8;
            uint16_t GT : 8;
        }   GTPR;
        uint16_t __RESERVED6;
    };

    enum { READ_BUFFER_SIZE = 256, WRITE_BUFFER_SIZE = 256 };

    System& mSystem;
    volatile USART* mBase;
    ClockControl* mClockControl;
    ClockControl::Clock mClock;
    uint32_t mSpeed;
    InterruptController::Line* mInterrupt;
    Dma::Stream* mDmaTx;
    Dma::Stream* mDmaRx;
    CircularBuffer mReadBuffer;
    CircularBuffer mWriteBuffer;

};

#endif // SERIAL_H
