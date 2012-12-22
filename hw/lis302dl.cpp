#include "lis302dl.h"

LIS302DL::LIS302DL(Spi<char> &spi) :
    mSpi(spi)
{
    mSpi.config(Spi<char>::MasterSlave::Master, Spi<char>::ClockPolarity::HighWhenIdle, Spi<char>::ClockPhase::SecondTransition, Spi<char>::Endianess::MsbFirst);
    mSpi.setSpeed(10000000);
}

void LIS302DL::enable()
{
    set(Register::Control1, DataRate100 | PowerUp | Range2G | EnableX | EnableY | EnableZ);
    set(Register::Control2, Spi4Wire | EnableFilter);
    set(Register::Control3, InterruptActiveHigh | InterruptPushPull | (static_cast<uint8_t>(InterruptConfig::DataReady) << Interrupt1ConfigShift) | (static_cast<uint8_t>(InterruptConfig::Click) << Interrupt2ConfigShift));
}

void LIS302DL::set(LIS302DL::Register, uint8_t value)
{
}
