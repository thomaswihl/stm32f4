#include "lis302dl.h"

LIS302DL::LIS302DL(Spi<char> &spi) :
    mSpi(spi)
{
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::HighWhenIdle, Spi<char>::ClockPhase::SecondTransition, Spi<char>::Endianess::MsbFirst);
    mSpi.setSpeed(10000000);
}

void LIS302DL::enable()
{
    mSpi.enable(Device::All);
    set(Register::Control1, DataRate100 | PowerUp | Range2G | EnableX | EnableY | EnableZ);
    set(Register::Control2, Spi4Wire | DisableFilter);
    set(Register::Control3, InterruptActiveHigh | InterruptPushPull | (static_cast<uint8_t>(InterruptConfig::DataReady) << Interrupt1ConfigShift) | (static_cast<uint8_t>(InterruptConfig::Click) << Interrupt2ConfigShift));
}

int8_t LIS302DL::x()
{
    return static_cast<int8_t>(get(Register::OutX));
}


int8_t LIS302DL::y()
{
    return static_cast<int8_t>(get(Register::OutY));
}

int8_t LIS302DL::z()
{
    return static_cast<int8_t>(get(Register::OutZ));
}

void LIS302DL::set(LIS302DL::Register reg, uint8_t value)
{
    char buf[2];
    buf[0] = WRITE | ADDR_CONST | static_cast<uint8_t>(reg);
    buf[1] = value;
    mSpi.write(buf, sizeof(buf));
}

uint8_t LIS302DL::get(LIS302DL::Register reg)
{
    char buf[2];
    buf[0] = READ | ADDR_CONST | static_cast<uint8_t>(reg);
    buf[1] = 0;
    mSpi.read(buf, sizeof(buf), nullptr);
    mSpi.write(buf, sizeof(buf));
    return buf[1];
}
