#include "sdio.h"

#include <cassert>


const char* const Sdio::STATUS_MSG[] =
{
    "Command response CRC error",
    "Data CRC error",
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

Sdio::Sdio(System::BaseAddress base, InterruptController::Line& irq, Dma::Stream& dma) :
    mBase(reinterpret_cast<volatile SDIO*>(base)),
    mIrq(irq),
    mDma(dma),
    mDebugLevel(0)
{
    static_assert(sizeof(SDIO) == 0x100, "Struct has wrong size, compiler problem.");
    enable(false);
    mDma.setCallback(this);
    mIrq.setCallback(this);
    mIrq.enable();
}

void Sdio::enable(bool enable)
{
    if (enable)
    {
        mBase->POWER.PWRCTRL = 3;
        mBase->CMD.bits.CPSMEN = 1;
        SDIO::__CLKCR clkcr;
        clkcr.value = 0;
        clkcr.bits.CLKEN = 1;
        clkcr.bits.PWRSAV = 0;
        mBase->CLKCR.value = clkcr.value;
        mBase->MASK = IM_MASK;
    }
    else
    {
        mBase->MASK = 0;
        reset();
    }
}

void Sdio::setClock(unsigned speed)
{
    unsigned div = (PLL_CLOCK + speed - 1) / speed;
    if (div <= 1)
    {
        mBase->CLKCR.bits.BYPASS = 1;
    }
    else
    {
        SDIO::__CLKCR clkcr;
        clkcr.value = mBase->CLKCR.value;
        clkcr.bits.BYPASS = 0;
        clkcr.bits.CLKDIV = div - 2;
        mBase->CLKCR.value = clkcr.value;
    }
    if (mDebugLevel > 0) printf("Setting clock to %luHz.\n", clock());
}

uint32_t Sdio::clock()
{
    if (mBase->CLKCR.bits.BYPASS)
    {
        return PLL_CLOCK;
    }
    else
    {
        return PLL_CLOCK / (mBase->CLKCR.bits.CLKDIV + 2);
    }
}

void Sdio::reset()
{
    mBase->POWER.PWRCTRL = 0;
    mBase->CLKCR.value = 0;
    mBase->ARG = 0;
    mBase->CMD.value = 0;
    mBase->RESP[0] = mBase->RESP[1] = mBase->RESP[2] = mBase->RESP[3] = 0;
    mBase->DTIMER = 0;
    mBase->DLEN = 0;
    mBase->DCTRL.value = 0;
    mBase->MASK = 0;
    mBase->ICR = IC_MASK;
    while (mBase->FIFOCNT > 0)
    {
        uint32_t data = mBase->FIFO[0];
    }
}

void Sdio::printHostStatus()
{
    uint32_t v = mBase->STA;
    printf("SDIO: %cCMD%i(%08lx), ", (mBase->RESP[0] & 0x00000020) ? 'A' : ' ', mBase->CMD.bits.CMD_WAIT & 0x3f, mBase->ARG);
    int count = 0;
    for (unsigned i = 0; i < sizeof(STATUS_MSG) / sizeof(STATUS_MSG[0]); ++i)
    {
        if ((v & (1 << i)) != 0)
        {
            if (count != 0) printf(",");
            printf(" '%s'", STATUS_MSG[i]);
            ++count;
        }
    }
    printf("\n");
}

void Sdio::setBusWidth(BusWidth width)
{
    if (width == BusWidth::OneDataLine) mBase->CLKCR.bits.WIDBUS = 0;
    else if (width == BusWidth::FourDataLines) mBase->CLKCR.bits.WIDBUS = 1;
    else if (width == BusWidth::EightDataLines) mBase->CLKCR.bits.WIDBUS = 2;
}

void Sdio::dmaCallback(Dma::Stream* stream, Dma::Stream::Callback::Reason reason)
{
}

void Sdio::interruptCallback(InterruptController::Index index)
{
    uint32_t status = mBase->STA;
    if (status & (CMDSENT | CMDREND | CCRCFAIL | CTIMEOUT))
    {
        // command complete, one way or the other
        if (mCompleteEvent != nullptr)
        {
            if (status & (CCRCFAIL | CTIMEOUT)) mCompleteEvent->setResult(System::Event::Result::CommandFail);
            else mCompleteEvent->setResult(System::Event::Result::CommandSuccess);
            System::instance()->postEvent(mCompleteEvent);
            mCompleteEvent = nullptr;
        }
    }
    if (status & (DCRCFAIL | DTIMEOUT | TXUNDERR | RXOVERR | DATAEND | STBITERR))
    {
        // transfer ended, one way or the other
        if (mCompleteEvent != nullptr)
        {
            if (status & (DCRCFAIL | DTIMEOUT | TXUNDERR | RXOVERR | STBITERR)) mCompleteEvent->setResult(System::Event::Result::DataFail);
            else mCompleteEvent->setResult(System::Event::Result::DataSuccess);
            System::instance()->postEvent(mCompleteEvent);
            mCompleteEvent = nullptr;
        }
    }
    mBase->ICR = status;
}

bool Sdio::sendCommand(uint8_t cmd, uint32_t arg, Response response, System::Event &completeEvent)
{
    bool longResponse = response == Response::Long || response == Response::LongNoCrc;
    bool waitResponse = response != Response::None;
    mIgnoreCrc = response == Response::ShortNoCrc || response == Response::LongNoCrc;
    mLastCommand = cmd & 0x3f;
    mCompleteEvent = &completeEvent;

    // check that no command is active
    if (mBase->STA & CMDACT) return false;
    // clear all status bits
    mBase->ICR = IC_MASK;
    if (mDebugLevel > 1) printf("SEND %i(%08lx) with %s\n", cmd, arg, toString(response));
    mBase->ARG = arg;
    mBase->CMD.bits.CMD_WAIT = (longResponse ? 0x80 : 0) | (waitResponse ? 0x40 : 0) | mLastCommand;
//    if (waitResponse)
//    {
//        // Wait for command to finish or timeout or CRC fail
//        while (mBase->STA.bits.CMDREND == 0 && mBase->STA.bits.CCRCFAIL == 0 && mBase->STA.bits.CTIMEOUT == 0) ;
//        if (mBase->STA.bits.CTIMEOUT)
//        {
//            printHostStatus();
//            return false;
//        }
//        if (mBase->STA.bits.CCRCFAIL && !ignoreCrc)
//        {
//            printHostStatus();
//            return false;
//        }
//    }
//    if (mDebugLevel > 1 && waitResponse) printf("RESP: %08lx\n", mBase->RESP[0]);
    return true;
}

uint32_t Sdio::shortResponse()
{
    return mBase->RESP[0];
}

void Sdio::longResponse(uint8_t *response)
{
    for (int i = 0; i < 4; ++i)
    {
        uint32_t v = mBase->RESP[i];
        for (int j = 0; j < 4; ++j) response[i * 4 + j] = v >> ((3 - j) * 8);
    }
}


const char *Sdio::toString(Sdio::Response response)
{
    switch (response)
    {
    case Sdio::Response::None:          return "None";
    case Sdio::Response::Short:         return "Short";
    case Sdio::Response::Long:          return "Long";
    case Sdio::Response::ShortNoCrc:    return "ShortNoCrc";
    case Sdio::Response::LongNoCrc:     return "LongNoCrc";
    }
    return "UNKNOWN_RESPONSE";
}

void Sdio::prepareTransfer(Direction direction, uint32_t* data, unsigned byteCount)
{
    mBase->DLEN = byteCount;
    SDIO::__DCTRL dctrl;
    dctrl.value = mBase->DCTRL.value;
    dctrl.bits.DTDIR = direction == Direction::Read ? 1 : 0;
    dctrl.bits.DTEN = 1;
    dctrl.bits.DTMODE = 0;
    mBase->DCTRL.value = dctrl.value;
    mDma.config((direction == Direction::Read) ? Dma::Stream::Direction::PeripheralToMemory : Dma::Stream::Direction::MemoryToPeripheral, false, true, Dma::Stream::DataSize::Word, Dma::Stream::DataSize::Word, Dma::Stream::BurstLength::Single, Dma::Stream::BurstLength::Single);
    mDma.configFifo(Dma::Stream::FifoThreshold::Disable);
    mDma.setAddress(Dma::Stream::End::Memory, reinterpret_cast<System::BaseAddress>(data));
    mDma.setAddress(Dma::Stream::End::Peripheral, reinterpret_cast<System::BaseAddress>(mBase->FIFO));
    mDma.setCallback(this);
    mDma.setFlowControl(Dma::Stream::FlowControl::Sdio);
    mDma.setTransferCount(byteCount / 4);
    mDma.start();
}

bool Sdio::setBlockSize(uint16_t blockSize)
{
    int bs;
    for (bs = 14; bs >= 0; --bs)
    {
        if (blockSize == (1 << bs)) break;
    }
    if (bs < 0) return false;
    mBase->DCTRL.bits.DBLOCKSIZE = bs;
    return true;
}

void Sdio::setDataTimeout(uint32_t clocks)
{
    mBase->DTIMER = clocks;
}
