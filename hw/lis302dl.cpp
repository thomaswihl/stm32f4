#include "lis302dl.h"

LIS302DL::LIS302DL(Spi &spi) :
    mSpi(spi)
{
    mSpi.config(Spi::MasterSlave::Master, Spi::ClockPolarity::HighWhenIdle, Spi::ClockPhase::SecondTransition, Spi::DataWidth::Eight, Spi::Endianess:MsbFirst);
    mSpi.setSpeed(10000000);
}

void LIS302DL::enable()
{
    set(Control1, DataRate100 | PowerUp | Range2G | EnableX | EnableY | EnableZ);
    set(Control2, Spi4Wire | EnableFilter);
    set(Control3, InterruptActiveHigh | InterruptPushPull | (InterruptConfig::DataReady << Interrupt1ConfigShift) | (InterruptConfig::Click << Interrupt1ConfigShift));
}

void LIS302DL::set(LIS302DL::Register, uint8_t value)
{
}
