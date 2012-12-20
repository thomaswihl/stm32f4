#ifndef LIS302DL_H
#define LIS302DL_H

#include "Spi.h"

class LIS302DL
{
public:
    LIS302DL(Spi& spi);

    void enable();
protected:
    Spi& mSpi;
    enum class InterruptConfig
    {
        Gnd = 0x00,
        FreeFallWakeUp1 = 0x01,
        FreeFallWakeUp2 = 0x02,
        FreeFallWakeUp1Or2 = 0x03,
        DataReady = 0x04,
        Click = 0x07,
    };

    enum Bits
    {
        // Control1
        DataRate100 = 0x00,
        DataRate400 = 0x80,
        PowerDown = 0x00,
        PowerUp = 0x40,
        Range2G = 0x00,
        Range8G = 0x20,
        EnableZ = 0x04,
        EnableY = 0x02,
        EnableX = 0x01,

        // Control2
        Spi4Wire = 0x00,
        Spi3Wire = 0x80,
        EnableFilter = 0x10,
        DisableFilter = 0x00,

        // Control3
        InterruptActiveHigh = 0x00,
        InterruptActiveLow = 0x80,
        InterruptPushPull = 0x00,
        InterruptOpenDrain = 0x40,
        Interrupt1ConfigShift = 0,
        Interrupt2ConfigShift = 3,

        // Status
        OverrunXYZ = 0x80,
        OverrunZ = 0x40,
        OverrunY = 0x20,
        OverrunX = 0x10,
        DataReadyXYZ = 0x08,
        DataReadyZ = 0x04,
        DataReadyY = 0x02,
        DataReadyX = 0x01,
    };

    enum class Register
    {
        WhoAmI = 0x0f,
        Control1 = 0x20,
        Control2 = 0x21,
        Control3 = 0x22,
        HpFilterReset = 0x23,
        Status = 0x27,
        OutX = 0x29,
        OutY = 0x2b,
        OutZ = 0x2d,
        FreeFallWakeUpConfig1 = 0x30,
        FreeFallWakeUpSource1 = 0x31,
        FreeFallWakeUpThreshold1 = 0x32,
        FreeFallWakeUpDuration1 = 0x33,
        FreeFallWakeUpConfig2 = 0x34,
        FreeFallWakeUpSource2 = 0x35,
        FreeFallWakeUpThreshold2 = 0x36,
        FreeFallWakeUpDuration2 = 0x37,
        ClickConfig = 0x38,
        ClickSource = 0x39,
        ClickThresholdYX = 0x3b,
        ClickThresholdZ = 0x3c,
        ClickTimeLimit = 0x3d,
        ClickLatency = 0x3e,
        ClickWindow = 0x3f,
    };

    void set(Register register, uint8_t value);
};

#endif // LIS302DL_H
