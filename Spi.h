#ifndef SPI_H
#define SPI_H

#include "System.h"
#include "ClockControl.h"
#include "Stream.h"
#include "Dma.h"
#include "Gpio.h"
#include "Device.h"

template<typename T>
class Spi : public Device, public ClockControl::Callback, public Stream<T>
{
public:
    Spi(System::BaseAddress base, ClockControl* clockControl, ClockControl::Clock clock);
    enum class MasterSlave { Master = 1, Slave = 0 };
    enum class ClockPolarity { LowWhenIdle = 0, HighWhenIdle = 1 };
    // Selects the transition for data capture
    enum class ClockPhase { FirstTransition = 0, SecondTransition = 1 };
    enum class Endianess { MsbFirst = 0, LsbFirst = 0 };

    uint32_t setSpeed(uint32_t maxSpeed);
    void setMasterSlave(MasterSlave masterSlave);
    void setClock(ClockPolarity clockPolarity, ClockPhase clockPhase);
    void setEndianess(Endianess endianess);

    void config(MasterSlave masterSlave, ClockPolarity clockPolarity, ClockPhase clockPhase, Endianess endianess);
    void configChipSelect(Gpio::Pin* chipSelect, bool activeLow);

    virtual void enable(Device::Part part);
    virtual void disable(Device::Part part);

    void configDma(Dma::Stream *write, Dma::Stream *read);
protected:
    virtual void clockCallback(ClockControl::Callback::Reason reason, uint32_t newClock);
    virtual void interruptCallback(InterruptController::Index index);

    virtual void dmaReadComplete(bool success);
    virtual void dmaWriteComplete(bool success);

private:
    struct SPI
    {
        struct __CR1
        {
            uint16_t CPHA : 1;
            uint16_t CPOL : 1;
            uint16_t MSTR : 1;
            uint16_t BR : 3;
            uint16_t SPE : 1;
            uint16_t LSBFIRST : 1;
            uint16_t SSI : 1;
            uint16_t SSM : 1;
            uint16_t RXONLY : 1;
            uint16_t DFF : 1;
            uint16_t CRCNEXT : 1;
            uint16_t CRCEN : 1;
            uint16_t BIDIOE : 1;
            uint16_t BIDIMODE : 1;
        }   CR1;
        uint16_t __RESERVED0;
        struct __CR2
        {
            uint16_t RXDMAEN : 1;
            uint16_t TXDMAEN : 1;
            uint16_t SSOE : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t FRF : 1;
            uint16_t ERRIE : 1;
            uint16_t RXNEIE : 1;
            uint16_t TXEIE : 1;
            uint16_t __RESERVED1 : 8;
        }   CR2;
        uint16_t __RESERVED1;
        struct __SR
        {
            uint16_t RXNE : 1;
            uint16_t TXE : 1;
            uint16_t CHSIDE : 1;
            uint16_t UDR : 1;
            uint16_t CRCERR : 1;
            uint16_t MODF : 1;
            uint16_t OVR : 1;
            uint16_t BSY : 1;
            uint16_t FRE : 1;
            uint16_t __RESERVED1 : 7;
        }   SR;
        uint16_t __RESERVED2;
        uint16_t DR;
        uint16_t __RESERVED3;
        uint16_t CRCPR;
        uint16_t __RESERVED4;
        uint16_t RXCRCR;
        uint16_t __RESERVED5;
        uint16_t TXCRCR;
        uint16_t __RESERVED6;
        struct __I2SCFGR
        {
            uint16_t CHLEN : 1;
            uint16_t DATLEN : 2;
            uint16_t CKPOL : 1;
            uint16_t I2SSTD : 2;
            uint16_t __RESERVED0 : 1;
            uint16_t PCMSYNC : 1;
            uint16_t I2SCFG : 2;
            uint16_t I2SE : 1;
            uint16_t I2SMOD : 1;
            uint16_t __RESERVED1 : 4;
        }   I2SCFGR;
        uint16_t __RESERVED7;
        struct __I2SPR
        {
            uint16_t I2SDIV : 8;
            uint16_t ODD : 1;
            uint16_t MCKOE : 1;
            uint16_t __RESERVED0 : 6;
        }   I2SPR;
        uint16_t __RESERVED8;
    };
    volatile SPI* mBase;
    ClockControl* mClockControl;
    ClockControl::Clock mClock;
    uint32_t mSpeed;
    Gpio::Pin* mChipSelect;
    bool mActiveLow;

    void select();
    void deselect();

    void waitTransmitComplete();
    void waitReceiveNotEmpty();

    virtual void readPrepare();
    virtual void readSync();
    virtual void readTrigger();
    virtual void readDone();

    virtual void writePrepare();
    virtual void writeSync();
    virtual void writeTrigger();
    virtual void writeDone();

};

extern template class Spi<char>;
extern template class Spi<uint16_t>;
#endif // SPI_H
