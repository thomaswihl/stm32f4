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
        mBase->DTIMER = 1000000;  // 4s timeout
        mBase->CMD.CPSMEN = 1;
    }
    else
    {
        mBase->CMD.CPSMEN = 0;
        mBase->POWER.PWRCTRL = 0;
    }
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

void Sdio::printHostStatus()
{
    printf("ST:");
    uint32_t v = mBase->STA.value;
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

void Sdio::resetCard()
{
    sendCommand(0, 0, false, true);
}

void Sdio::initCard()
{
    resetCard();
    Result result = interfaceCondition();
    result = initializeCard(true);
}

Sdio::Result Sdio::interfaceCondition()
{
    union
    {
        struct
        {
            uint32_t checkPattern : 8;
            uint32_t voltageSupplied : 4;
            uint32_t __RESERVED : 20;
        }   bits;
        uint32_t value;
    }   arg;
    arg.value = 0;
    arg.bits.checkPattern = CHECK_PATTERN;
    arg.bits.voltageSupplied = 1; // 2.7 - 3.6V
    sendCommand(8, arg.value);
    if (mBase->RESP[0] != arg.value) return Result::Error;
    return Result::Ok;
}

Sdio::Result Sdio::initializeCard(bool hcSupport)
{
    union
    {
        struct
        {
            uint32_t __RESERVED0 : 8;
            uint32_t OCR : 16;
            uint32_t switchTo1_8V : 1;
            uint32_t __RESERVED1 : 3;
            uint32_t xcPowerControl : 1;
            uint32_t uhsSupport : 1;
            uint32_t hcSupport : 1;
            uint32_t busy : 1;
        }   bits;
        uint32_t value;
    }   arg;
    arg.value = 0;
    arg.bits.OCR = ocrFromVoltage(30);  // 3.0V
    arg.bits.hcSupport = hcSupport ? 1 : 0;
    Result result;
    do
    {
        result = sendAppCommand(41, arg.value, true, true);
        if (result != Result::Ok) return result;
        printOcr();
    }   while ((mBase->RESP[0] & 0x80000000) == 0);
    return result;
}

Sdio::Result Sdio::sendCommand(uint8_t cmd, uint32_t arg, bool waitResponse, bool ignoreCrc)
{
    Result result;
    int retry = 5;
    do
    {
        result = Result::Ok;
        printf("SEND %i(%08x)\n", cmd, arg);
        mBase->ARG = arg;
        mBase->ICR.value = 0x00c007ff;
        mBase->CMD.CMD_WAIT = (waitResponse ? 0x40 : 0) | cmd;
        if (waitResponse)
        {
            while (mBase->STA.bits.CMDREND == 0 && mBase->STA.bits.CCRCFAIL == 0 && mBase->STA.bits.CTIMEOUT == 0) ;
            if (mBase->STA.bits.CTIMEOUT) result = Result::Timeout;
            if (mBase->STA.bits.CCRCFAIL && !ignoreCrc) result = Result::CrcError;
        }
        --retry;
    }   while (result != Result::Ok && retry >= 0);
    if (result != Result::Ok) printHostStatus();
    return result;
}

Sdio::Result Sdio::sendAppCommand(uint8_t cmd, uint32_t arg, bool waitResponse, bool ignoreCrc)
{
    mBase->ARG = 0;
    Result result = sendCommand(55, 0);
    if (result == Result::Ok)
    {
        if (checkCardStatus(true))
        {
            result = sendCommand(cmd, arg, waitResponse, ignoreCrc);
        }
        else
        {
            result = Result::Error;
        }
    }
    return result;
}

bool Sdio::checkCardStatus(bool expectAppCmd)
{
    static const uint32_t AKE_SEQ_ERROR = 1 << 3;
    static const uint32_t APP_CMD = 1 << 5;
    static const uint32_t READY_FOR_DATA = 1 << 8;
    static const uint32_t CURRENT_STATE_MASK = 15 << 9;
    static const int CURRENT_STATE_SHIFT = 9;
    static const uint32_t ERASE_RESET = 1 << 13;
    static const uint32_t CARD_ECC_DISABLED = 1 << 14;
    static const uint32_t WP_ERASE_SKIP = 1 << 15;
    static const uint32_t CSD_OVERWRITE = 1 << 16;
    static const uint32_t ERROR = 1 << 19;
    static const uint32_t CC_ERROR = 1 << 20;
    static const uint32_t CARD_ECC_FAILED = 1 << 21;
    static const uint32_t ILLEGAL_COMMAND = 1 << 22;
    static const uint32_t COM_CRC_ERROR = 1 << 23;
    static const uint32_t LOCK_UNLOCK_FAILED = 1 << 24;
    static const uint32_t CARD_IS_LOCKED = 1 << 25;
    static const uint32_t WP_VIOLATION = 1 << 26;
    static const uint32_t ERASE_PARAM = 1 << 27;
    static const uint32_t ERASE_SEQ_ERROR = 1 << 28;
    static const uint32_t BLOCK_LEN_ERROR = 1 << 29;
    static const uint32_t ADDRESS_ERROR = 1 << 30;
    static const uint32_t OUT_OF_RANGE = 1 << 31;
    static const char* const CURRENT_STATE[16] = { "Idle", "Ready", "Identification", "Standby", "Transfer", "Data", "Receive", "Programm", "Disabled", "9", "10", "11", "12", "13", "14", "15" };
    static const char* const STATE[32] = { 0, 0, 0, "AKE_SEQ_ERROR", 0, 0, 0, 0,
                                           "READY_FOR_DATA", 0, 0, 0, 0, "ERASE_RESET", "CARD_ECC_DISABLED", "WP_ERASE_SKIP",
                                           "CSD_OVERWRITE", 0, 0, "ERROR", "CC_ERROR", "CARD_ECC_FAILED", "ILLEGAL_COMMAND", "COM_CRC_ERROR",
                                           "LOCK_UNLOCK_FAILED", "CARD_IS_LOCKED", "WP_VIOLATION", "ERASE_PARAM", "ERASE_SEQ_ERROR", "BLOCK_LEN_ERROR", "ADDRESS_ERROR", "OUT_OF_RANGE" };
    uint32_t status = mBase->RESP[0];
    if ((status & (AKE_SEQ_ERROR | WP_ERASE_SKIP | CSD_OVERWRITE | ERROR |
            CC_ERROR | CARD_ECC_FAILED | ILLEGAL_COMMAND | COM_CRC_ERROR |
            COM_CRC_ERROR | LOCK_UNLOCK_FAILED | WP_VIOLATION | ERASE_PARAM |
            ERASE_SEQ_ERROR | BLOCK_LEN_ERROR | ADDRESS_ERROR | OUT_OF_RANGE)) != 0)
    {
        printf("SD ERROR:");
        for (int i = 0; i < 32; ++i)
        {
            if ((status & (1 << i)) != 0 && STATE[i] != 0) printf(" %s", STATE[i]);
        }
        printf("\nCard state is %s\n", CURRENT_STATE[(status & CURRENT_STATE_MASK) >> CURRENT_STATE_SHIFT]);
        return false;
    }
    else
    {
        bool isAppCmd = (status & APP_CMD) != 0;
        if (isAppCmd != expectAppCmd)
        {
            printf("SD(%s): Expected %s command but card says %s command.\n", CURRENT_STATE[(status & CURRENT_STATE_MASK) >> CURRENT_STATE_SHIFT], expectAppCmd ? "app" : "regular", isAppCmd ? "app" : "regular");
            return false;
        }
    }

    return true;
}

uint32_t Sdio::ocrFromVoltage(int volt)
{
    uint32_t ocr = 0;
    int minVolt = 35;
    for (int i = 15; i >= 7; --i)
    {
        if (volt >= minVolt && volt <= (minVolt + 1)) ocr |= 1 << i;
        minVolt -= 1;
    }
    return ocr;
}

void Sdio::printOcr()
{
    uint32_t ocr = mBase->RESP[0];
    printf("Card is ");
    if ((ocr & 0x80000000) == 0) printf("BUSY\n");
    else
    {
        printf("READY, capacity is ");
        if (ocr & 0x40000000) printf("HC/XC");
        else printf("SC");
        int first = 0, last = 0;
        for (int i = 15; i < 24; ++i)
        {
            if ((ocr & (1 << i)) != 0)
            {
                if (first == 0) first = i - 15 + 27;
                last = i - 15 + 28;
            }
        }
        printf(" and supports %i.%i-%i.%iV\n", first / 10, first % 10, last / 10, last % 10);
    }
}
