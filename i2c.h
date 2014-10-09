#include "ClockControl.h"
#include "Device.h"

#ifndef I2C_H
#define I2C_H

class I2C : public Device, public ClockControl::Callback
{
public:
    enum class Mode { Standard, FastDuty2, FastDuty16by9 };
    enum class AddressMode { SevenBit, TenBit };
    class Chip;

    struct Transfer
    {
        const uint8_t* mWriteData;
        unsigned mWriteLength;
        uint8_t* mReadData;
        unsigned mReadLength;
        System::Event* mEvent;
        Chip* mChip;
    };

    class Chip
    {
    public:
        Chip(I2C& i2c) :
            mI2C(i2c)
        { }

        virtual bool transfer(Transfer* transfer) { transfer->mChip = this; return mI2C.transfer(transfer); }
        virtual void prepare() { }

        uint32_t maxSpeed() const { return mMaxSpeed; }
        void setMaxSpeed(const uint32_t &maxSpeed) { mMaxSpeed = maxSpeed; }
        Mode mode() const { return mMode; }
        void setMode(const Mode &mode) { mMode = mode; }
        uint16_t address() const { return mAddress; }
        void setAddress(const uint16_t &address) { mAddress = address; }
        AddressMode addressMode() const { return mAddressMode; }
        void setAddressMode(const AddressMode &addressMode) { mAddressMode = addressMode; }

    private:
        I2C& mI2C;
        uint32_t mMaxSpeed;
        Mode mMode;
        uint16_t mAddress;
        AddressMode mAddressMode;
    };


    I2C(System::BaseAddress base, ClockControl* clockControl, ClockControl::Clock clock);

    void enable(Part part);
    void disable(Part part);
    void setAddress(uint16_t address, AddressMode mode);

    bool transfer(Transfer* transfer);
    void configDma(Dma::Stream *write, Dma::Stream *read);
    void configInterrupt(InterruptController::Line *event, InterruptController::Line *error);

protected:
    void dmaReadComplete();
    void dmaWriteComplete();

    void clockCallback(ClockControl::Callback::Reason reason, uint32_t clock);
    void interruptCallback(InterruptController::Index index);

private:
    struct IIC
    {
        struct __CR1
        {
            uint16_t PE : 1;
            uint16_t SMBUS : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t SMBTYPE : 1;
            uint16_t ENARP : 1;
            uint16_t ENPEC : 1;
            uint16_t ENGC : 1;
            uint16_t NOSTRETCH : 1;
            uint16_t START : 1;
            uint16_t STOP : 1;
            uint16_t ACK : 1;
            uint16_t POS : 1;
            uint16_t PEC : 1;
            uint16_t ALERT : 1;
            uint16_t __RESERVED1 : 1;
            uint16_t SWRST : 1;
        }   CR1;
        uint16_t __RESERVED0;
        struct __CR2
        {
            uint16_t FREQ : 6;
            uint16_t __RESERVED0 : 2;
            uint16_t ITERREN : 1;
            uint16_t ITEVTEN : 1;
            uint16_t ITBUFEN : 1;
            uint16_t DMAEN : 1;
            uint16_t LAST : 1;
            uint16_t __RESERVED1 : 3;
        }   CR2;
        uint16_t __RESERVED1;
        struct __OAR1
        {
            uint16_t ADD : 10;
            uint16_t __RESERVED1 : 5;
            uint16_t ADDMODE : 1;
        }   OAR1;
        uint16_t __RESERVED2;
        struct __OAR2
        {
            uint16_t ADD : 8;
            uint16_t __RESERVED1 : 8;
        }   OAR2;
        uint16_t __RESERVED3;
        uint8_t DR;
        uint8_t __RESERVED3A;
        uint16_t __RESERVED4;
        struct __SR1
        {
            uint16_t SB : 1;
            uint16_t ADDR : 1;
            uint16_t BTF : 1;
            uint16_t ADD10 : 1;
            uint16_t STOPF : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t RxNE : 1;
            uint16_t TxE : 1;
            uint16_t BERR : 1;
            uint16_t ARLO : 1;
            uint16_t AF : 1;
            uint16_t OVR : 1;
            uint16_t PECERR : 1;
            uint16_t __RESERVED1 : 1;
            uint16_t TIMEOUT : 1;
            uint16_t SMBALERT : 1;
        }   SR1;
        uint16_t __RESERVED5;
        struct __SR2
        {
            uint16_t MSL : 1;
            uint16_t BUSY : 1;
            uint16_t TRA : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t GENCALL : 1;
            uint16_t SMBDEFAULT : 1;
            uint16_t SMBHOST : 1;
            uint16_t DUALF : 1;
            uint16_t PEC : 8;
        }   SR2;
        uint16_t __RESERVED6;
        struct __CCR
        {
            uint16_t CCR : 12;
            uint16_t __RESERVED0 : 2;
            uint16_t DUTY : 1;
            uint16_t FS : 1;
        }   CCR;
        uint16_t __RESERVED7;
        struct __TRISE
        {
            uint16_t TRISE : 6;
            uint16_t __RESERVED0 : 10;
        }   TRISE;
        uint16_t __RESERVED8;
    };
    volatile IIC* mBase;
    ClockControl* mClockControl;
    ClockControl::Clock mClock;
    CircularBuffer<Transfer*> mTransferBuffer;
    InterruptController::Line *mEvent;
    InterruptController::Line *mError;
    Transfer* mActiveTransfer;

    void setSpeed(uint32_t maxSpeed, Mode mode);
    void nextTransfer();

};

#endif // I2C_H
