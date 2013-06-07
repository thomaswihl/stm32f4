#include "sdio.h"

const char* const Sdio::STATUS_MSG[] =
{
    "Command response CRC error",
    "Data CDC error",
    "Command response timeout",
    "Data timeout",
    "Transmit FIFO underrun",
    "Receive FIFO underrun",
    "Command response received",
    "Command sent",
    "Data end",
    "Start bit error",
    "Data block transferred",
    "Command in progress",
    "Data transmit in progress",
    "Data receive in progress",
    "Transmit FIFO half empty",
    "Receive FIFO half full",
    "Transmit FIFO full",
    "Receive FIFO full",
    "Transmit FIFO empty",
    "Receive FIFO empty",
    "Data available in transmit FIFO",
    "Data available in receive FIFO",
    "SDIO interrupt",
    "CE-ATA command completion signal received"
};

Sdio::Sdio(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile SDIO*>(base))
{
    static_assert(sizeof(SDIO) == 0x84, "Struct has wrong size, compiler problem.");
}

void Sdio::enable(bool enable)
{
    if (enable)
    {
        setClock(CLOCK_IDENTIFICATION);
        mBase->POWER.PWRCTRL = 3;
        mBase->CLKCR.CLKEN = 1;
        mBase->CLKCR.PWRSAV = 0;
    }
    else mBase->POWER.PWRCTRL = 0;
}

void Sdio::setClock(unsigned clock)
{
    unsigned div = (48000000 + clock - 1) / clock;
    if (div <= 1)
    {
        mBase->CLKCR.BYPASS = 1;
    }
    else
    {
        mBase->CLKCR.BYPASS = 0;
        mBase->CLKCR.CLKDIV = div - 2;
    }
}

void Sdio::printStatus()
{
    uint32_t v = mBase->STA.value;
    for (unsigned i = 0; i < sizeof(STATUS_MSG) / sizeof(STATUS_MSG[0]); ++i)
    {
        if ((v & (1 << i)) != 0) printf("%s ", STATUS_MSG[i]);
    }
}
