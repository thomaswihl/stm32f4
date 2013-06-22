#include "sdcard.h"

#include "assert.h"

SdCard::SdCard(Sdio& sdio, int supplyVoltage) :
    mSdio(sdio),
    mVolt(supplyVoltage),
    mDebugLevel(1)
{
}


void SdCard::reset()
{
    mSdio.reset();
    memset(&mCardInfo, 0, sizeof(mCardInfo));
    mSdio.enable(true);
    mSdio.setClock(CLOCK_IDENTIFICATION);
    mSdio.sendCommand(0, 0, Sdio::Response::None);
}

bool SdCard::init()
{
    reset();
    // if the card responds it is a v2 or later
    bool v2 = interfaceCondition();
    // Send ACMD41, to check voltage ranges
    if (!initializeCard(v2))
    {
        System::instance()->printWarning("SDIO", "Card not supported or not a SD/SDHC/SDXC card.");
        return false;
    }
    if (!getCardIdentifier())
    {
        System::instance()->printWarning("SDIO", "Can't read CID.");
        return false;
    }
    if (!getRelativeCardAddress())
    {
        System::instance()->printWarning("SDIO", "Can't read RCA.");
        return false;
    }
    if (!getCardSpecificData())
    {
        System::instance()->printWarning("SDIO", "Can't read CSD.");
        return false;
    }
    mSdio.setClock(mCardInfo.mTransferRate);
    if (!selectCard())
    {
        System::instance()->printWarning("SDIO", "Can't select card.");
        return false;
    }
    if (!getCardStatus())
    {
        System::instance()->printWarning("SDIO", "Can't read status.");
        return false;
    }
    if (!getCardConfiguration())
    {
        System::instance()->printWarning("SDIO", "Can't read configuration.");
        return false;
    }
    if (!setBusWidth())
    {
        System::instance()->printWarning("SDIO", "Can't set bus width.");
        return false;
    }
    return true;
}

bool SdCard::interfaceCondition() // CMD8
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
    if (!mSdio.sendCommand(8, arg.value, Sdio::Response::Short)) return false;
    if (mSdio.shortResponse() != arg.value) return false;
    return true;
}

bool SdCard::initializeCard(bool hcSupport)
{
    static const uint32_t READY_BIT = 0x80000000;
    static const uint32_t HIGH_CAPACITY_BIT = 0x40000000;
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
    arg.bits.OCR = ocrFromVoltage(mVolt);
    arg.bits.hcSupport = hcSupport ? 1 : 0;

    uint32_t response;
    int timeout = 100;  // max of 100 * 10ms = 1s
    do
    {
        if (!sendAppCommand(41, arg.value, Sdio::Response::ShortNoCrc)) return false;
        response = mSdio.shortResponse();
        if ((response & READY_BIT) != 0) break;
        // give the card some time...
        System::instance()->usleep(10000);
        --timeout;
    }   while (timeout >= 0);
    if ((response & READY_BIT) == 0) return false;
    else mCardInfo.mHc = (response & HIGH_CAPACITY_BIT) != 0;
    if (mDebugLevel > 1) printOcr();
    return true;
}

bool SdCard::getCardIdentifier()
{
    union
    {
        struct
        {
            uint8_t MID;
            uint8_t OID[2];
            uint8_t PNM[5];
            uint8_t PRV;
            uint8_t PSN[4];
            uint8_t MDT[2];
            uint8_t CRC;
        }   bits;
        uint8_t value[16];
    }   cid;

    if (mSdio.sendCommand(2, 0, Sdio::Response::Long))
    {
        mSdio.longResponse(cid.value);
        if (mDebugLevel > 0)
        {
            printf("   Manufacturer ID : %u\n", cid.bits.MID);
            printf("OEM/Application ID : %c%c\n", cid.bits.OID[0], cid.bits.OID[1]);
            printf("      Product name : %c%c%c%c%c\n", cid.bits.PNM[0], cid.bits.PNM[1], cid.bits.PNM[2], cid.bits.PNM[3], cid.bits.PNM[4]);
            printf("  Product revision : %i.%i\n", cid.bits.PRV >> 4, cid.bits.PRV & 0xf);
            printf("    Product serial : %u\n", cid.bits.PSN[0] << 24 | cid.bits.PSN[1] << 16 | cid.bits.PSN[2] << 8 | cid.bits.PSN[3]);
            printf("Manufacturing Date : %u.%u\n", cid.bits.MDT[1] & 0xf, ((cid.bits.MDT[1] >> 4) | ((cid.bits.MDT[0] & 0x0f) << 4)) + 2000);
        }
        return true;
    }
    return false;
}

bool SdCard::getRelativeCardAddress()
{
    if (mSdio.sendCommand(3, 0, Sdio::Response::Short))
    {
        mCardInfo.mRca = mSdio.shortResponse() & 0xffff0000;
        if (mDebugLevel > 1) printf("RCA is %04x\n", mCardInfo.mRca >> 16);
        return checkCardStatus(mSdio.shortResponse() & 0x0000ffff);
    }
    return false;
}

bool SdCard::getCardSpecificData()
{
    static const int CSD_LEN = 16;
    static const int TIME_TABLE[16] = { 0, 1000, 1200, 1300, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 7000, 8000 };
    static const int CURRENT_MIN[8] = { 0, 1, 5, 10, 25, 35, 60, 100 };
    static const int CURRENT_MAX[8] = { 1, 5, 10, 25, 35, 45, 80, 200 };
    static const char* const FILE_FORMAT[4] = { "HDD", "Floppy", "Universal", "Others/Unknown" };

    if (mSdio.sendCommand(9, mCardInfo.mRca, Sdio::Response::Long))
    {
        uint8_t csd[16];
        mSdio.longResponse(csd);

        // CSD_STRUCTURE
        int csdVersion = getBits(csd, CSD_LEN, 127, 126) + 1;
        if (mDebugLevel > 1) printf("\nCSD (v%i):\n---------\n", csdVersion);

        // N_AC(max) = 100 * ((TAAC * f_interface) + (100 * NSAC))
        mCardInfo.mTaac = pow10(getBits(csd, CSD_LEN, 114, 112) - 3) * TIME_TABLE[getBits(csd, CSD_LEN, 118, 115)];
        mCardInfo.mNsac = getBits(csd, CSD_LEN, 111, 104);
        if (mDebugLevel > 1) printf("T_AAC is %uns, N_SAC is %u.\n", mCardInfo.mTaac, mCardInfo.mNsac);

        // TRAN_SPEED
        mCardInfo.mTransferRate = 100 * pow10(getBits(csd, CSD_LEN, 98, 96)) * TIME_TABLE[getBits(csd, CSD_LEN, 102, 99)];
        if (mDebugLevel > 0) printf("Maximum transfer rate is %u Hz.\n", mCardInfo.mTransferRate);

        // CCC
        mCardInfo.mCommandClass = getBits(csd, CSD_LEN, 95, 84);
        if (mDebugLevel > 1)
        {
            static const char* const COMMAND_CLASS[] = { "basic", 0, "block read", 0, "block write", "erase", "write protection", "lock card", "application specific", "I/O mode", "switch", "extension" };
            printf("Supported command classes:");
            for (int i = 0; i <= 11; ++i)
            {
                if (mCardInfo.mCommandClass & (1 << i))
                {
                    if (COMMAND_CLASS[i] != 0) printf(" '%s'", COMMAND_CLASS[i]);
                    else printf(" %i", i);
                }
            }
            printf("\n");
        }

        // READ_BL_LEN
        mCardInfo.mReadBlockSize = 1 << getBits(csd, CSD_LEN, 83, 80);
        // READ_BL_PARITAL
        mCardInfo.mPartialBlockRead = getBits(csd, CSD_LEN, 79, 79);
        // READ_BL_MISALIGN
        mCardInfo.mReadBlockMisalign = getBits(csd, CSD_LEN, 77, 77);
        // WRITE_BL_LEN
        mCardInfo.mWriteBlockSize = 1 << getBits(csd, CSD_LEN, 25, 22);
        // WRITE_BL_PARTIAL
        mCardInfo.mPartialBlockWrite = getBits(csd, CSD_LEN, 21, 21);
        // WRITE_BL_MISALIGN
        mCardInfo.mWriteBlockMisalign = getBits(csd, CSD_LEN, 78, 78);
        if (mDebugLevel > 1)
        {
            printf("Max block size is %u for reading and %u for writing.\n", mCardInfo.mReadBlockSize, mCardInfo.mWriteBlockSize);
            printf("Partial reading is %ssupported, partial writing is %ssupported.\n", mCardInfo.mPartialBlockRead ? "" : "NOT ", mCardInfo.mPartialBlockWrite ? "" : "NOT ");
            printf("Misaligned reading is %ssupported, misaligned writing is %ssupported.\n", mCardInfo.mReadBlockMisalign ? "" : "NOT ", mCardInfo.mWriteBlockMisalign ? "" : "NOT ");
        }

        // DSR_IMP
        mCardInfo.mDriverStageImplemented = getBits(csd, CSD_LEN, 76, 76);

        if (csdVersion == 1)
        {
            // C_SIZE & C_SIZE_MULT
            mCardInfo.mBlockCount = (getBits(csd, CSD_LEN, 73, 62) + 1) * (1 << (getBits(csd, CSD_LEN, 49, 47) + 2));
        }
        else if (csdVersion == 2)
        {

            mCardInfo.mBlockCount = (getBits(csd, CSD_LEN, 69, 48) + 1) * 1024;
        }
        if (mDebugLevel > 0) printf("Size: %u blocks x %u = %luMB.\n", mCardInfo.mBlockCount, mCardInfo.mReadBlockSize, static_cast<uint32_t>(((static_cast<uint64_t>(mCardInfo.mBlockCount) * static_cast<uint64_t>(mCardInfo.mReadBlockSize)) >> 20)));
        if (csdVersion == 1)
        {
            // VDD_R_CURR_MIN
            mCardInfo.mReadCurrentMin = CURRENT_MIN[getBits(csd, CSD_LEN, 61, 59)];
            // VDD_R_CURR_MAX
            mCardInfo.mReadCurrentMax = CURRENT_MAX[getBits(csd, CSD_LEN, 58, 56)];
            // VDD_W_CURR_MIN
            mCardInfo.mWriteCurrentMin = CURRENT_MIN[getBits(csd, CSD_LEN, 55, 53)];
            // VDD_W_CURR_MAX
            mCardInfo.mWriteCurrentMax = CURRENT_MAX[getBits(csd, CSD_LEN, 52, 50)];
            if (mDebugLevel > 1) printf("Card requires [%u, %u]mA for reading and [%u, %u]mA for writing.\n", mCardInfo.mReadCurrentMin, mCardInfo.mReadCurrentMax, mCardInfo.mWriteCurrentMin, mCardInfo.mWriteCurrentMax);
        }
        // ERASE_BLK_EN
        mCardInfo.mEraseSingleBlock = getBits(csd, CSD_LEN, 46, 46);
        // SECTOR_SIZE = number of write blocks
        mCardInfo.mEraseBlockSize = (getBits(csd, CSD_LEN, 45, 39) + 1) * mCardInfo.mWriteBlockSize;
        if (mDebugLevel > 1) printf("Card can erase sectors in multiple of %u bytes.\n", mCardInfo.mEraseSingleBlock ? 512 : mCardInfo.mEraseBlockSize);
        // WP_GRP_SIZE
        mCardInfo.mWriteProtectGroupSize = (getBits(csd, CSD_LEN, 38, 32) + 1) * mCardInfo.mEraseBlockSize;
        // WP_GRP_ENABLE
        mCardInfo.mWriteProtectGroupEnabled = getBits(csd, CSD_LEN, 31, 31);
        if (mDebugLevel > 1)
        {
            if (mCardInfo.mWriteProtectGroupEnabled) printf("Card can write protect groups of size %i.\n", mCardInfo.mWriteProtectGroupSize);
            else printf("Card can NOT write protect groups.\n");
        }
        // R2W_FACTOR
        mCardInfo.mReadToWriteFactor = 1 << getBits(csd, CSD_LEN, 28, 26);
        if (mDebugLevel > 1) printf("Writing is %i times slower than reading.\n", mCardInfo.mReadToWriteFactor);
        // FILE_FORMAT_GRP
        mCardInfo.mFileFormatGroup = getBits(csd, CSD_LEN, 15, 15);
        // COPY
        mCardInfo.mCopy = getBits(csd, CSD_LEN, 14, 14);
        if (mDebugLevel > 1) printf("Card is %s.\n", mCardInfo.mCopy ? "copy" : "original");
        // PERM_WRITE_PROTECT
        mCardInfo.mPermanentlyWriteProtected = getBits(csd, CSD_LEN, 13, 13);
        if (mDebugLevel > 1) printf("Card is %spermanently write protected.\n", mCardInfo.mPermanentlyWriteProtected ? "" : "NOT ");
        // TMP_WRITE_PROTECT
        mCardInfo.mTemporarilyWriteProtected = getBits(csd, CSD_LEN, 12, 12);
        if (mDebugLevel > 1) printf("Card is %stemporaily write protected.\n", mCardInfo.mTemporarilyWriteProtected ? "" : "NOT ");
        // FILE_FORMAT
        mCardInfo.mFileFormat = getBits(csd, CSD_LEN, 11, 10);
        if (mDebugLevel > 1) printf("File format is %s.\n", mCardInfo.mFileFormatGroup ? "Reserved" : FILE_FORMAT[mCardInfo.mFileFormat]);
        return true;
    }
    return false;
}

bool SdCard::selectCard(bool select)
{
    if (mSdio.sendCommand(7, mCardInfo.mRca, Sdio::Response::ShortNoCrc) && checkCardStatus(mSdio.shortResponse())) return true;
    return false;
}

bool SdCard::getCardStatus()
{
    if (mSdio.sendCommand(13, mCardInfo.mRca, Sdio::Response::Short) && checkCardStatus(mSdio.shortResponse())) return true;
    return false;
}

bool SdCard::getCardConfiguration()
{
    static const int SCR_LEN = 8;
    if (!setBlockSize(SCR_LEN)) return false;
    if (!mSdio.sendCommand(55, mCardInfo.mRca, Sdio::Response::Short) || !checkCardStatus(mSdio.shortResponse())) return false;

    uint8_t data[SCR_LEN];
    mSdio.prepareTransfer(Sdio::Direction::Read, reinterpret_cast<uint32_t*>(data), SCR_LEN);
    if (!mSdio.sendCommand(51, 0, Sdio::Response::Short) || !checkCardStatus(mSdio.shortResponse())) return false;

//    while (mBase->STA.bits.DBCKEND == 0 && mBase->STA.bits.DCRCFAIL == 0 && mBase->STA.bits.DTIMEOUT == 0 && mBase->STA.bits.STBITERR == 0)
//    {
//    }
//    while (mBase->STA.bits.RXDVAL != 0 && count < SCR_LEN)
//    {
//        uint32_t d = mBase->FIFO[0];
//        for (int i = 0; i < 4; ++i) data[count++] = d >> (8 * i);
//    }
//    if (mBase->STA.bits.DCRCFAIL != 0) return Result::CrcError;
//    if (mBase->STA.bits.DTIMEOUT != 0) return Result::Timeout;
//    if (mBase->STA.bits.STBITERR != 0) return Result::Error;
//    if (mBase->STA.bits.DBCKEND == 0) return Result::Error;
//    if (count != SCR_LEN || mBase->STA.bits.RXDVAL != 0) return Result::Error;
    if (getBits(data, SCR_LEN, 63, 60) == 0)
    {
//        printf("SCR:");
//        for (int i = 0; i < SCR_LEN; ++i) printf(" %02x",data[i]);
//        printf("\n");
        uint32_t sdSpec = getBits(data, SCR_LEN, 59, 56);
        mCardInfo.mDataStatusAfterErase = getBits(data, SCR_LEN, 55, 55);
        //uint32_t security = getBits(data, SCR_LEN, 54, 52);
        uint32_t busWidth = getBits(data, SCR_LEN, 51, 48);
        uint32_t sdSpec3 = getBits(data, SCR_LEN, 47, 47);
        //uint32_t extendedSecurity = getBits(data, SCR_LEN, 46, 43);
        mCardInfo.mSetBlockCountSupport = getBits(data, SCR_LEN, 33, 33);
        mCardInfo.mSpeedClassControlSupport = getBits(data, SCR_LEN, 32, 32);

        if (sdSpec == 0) mCardInfo.mSpecVersion = 100;
        else if (sdSpec == 1) mCardInfo.mSpecVersion = 110;
        else if (sdSpec == 2)
        {
            if (sdSpec3 == 0) mCardInfo.mSpecVersion = 200;
            else mCardInfo.mSpecVersion = 300;
        }
        else if (sdSpec == 3) mCardInfo.mSpecVersion = 400;
        else mCardInfo.mSpecVersion = 400;

        if ((busWidth & 4) == 4) mCardInfo.mBusWidth = 4;
        else mCardInfo.mBusWidth = 1;
        if (mDebugLevel > 0) printf("Card supports spec v%i.%02i and %i bit data bus.\n", mCardInfo.mSpecVersion / 100, mCardInfo.mSpecVersion % 100, mCardInfo.mBusWidth);
    }
    return true;
}

bool SdCard::setBusWidth()
{
    if (sendAppCommand(6, (mCardInfo.mBusWidth == 4) ? 2 : 0, Sdio::Response::Short) && checkCardStatus(mSdio.shortResponse()))
    {
        if (mCardInfo.mBusWidth == 1) mSdio.setBusWidth(Sdio::BusWidth::OneDataLine);
        else if (mCardInfo.mBusWidth == 4) mSdio.setBusWidth(Sdio::BusWidth::FourDataLines);
        else if (mCardInfo.mBusWidth == 8) mSdio.setBusWidth(Sdio::BusWidth::EightDataLines);
        return true;
    }
    return false;
}

bool SdCard::setBlockSize(uint16_t blockSize)
{
    return mSdio.sendCommand(16, blockSize, Sdio::Response::Short) && checkCardStatus(mSdio.shortResponse());
}

bool SdCard::sendAppCommand(uint8_t cmd, uint32_t arg, Sdio::Response response)
{
    if (mSdio.sendCommand(55, mCardInfo.mRca, Sdio::Response::Short) && checkCardStatus(mSdio.shortResponse()))
    {
        return mSdio.sendCommand(cmd, arg, response);
    }
    return false;
}

bool SdCard::checkCardStatus(uint32_t status)
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
    bool isAppCmd = (status & APP_CMD) != 0;
    if (mDebugLevel > 1) printf("SD(%s):%s:%s%s%s%s.\n", CURRENT_STATE[(status & CURRENT_STATE_MASK) >> CURRENT_STATE_SHIFT],
                                isAppCmd ? "APP_CMD" : "REGULAR_CMD",
                                ((status & READY_FOR_DATA) != 0) ? " 'READY_FOR_DATA'" : "",
                                ((status & ERASE_RESET) != 0) ? " 'ERASE_RESET'" : "",
                                ((status & CARD_ECC_DISABLED) != 0) ? " 'CARD_ECC_DISABLED'" : "",
                                ((status & CARD_IS_LOCKED) != 0) ? " 'CARD_IS_LOCKED'" : ""
                           );

    return true;
}

uint32_t SdCard::ocrFromVoltage(int volt)
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

void SdCard::voltageFromOcr(uint32_t ocr, int& minVoltage, int& maxVoltage)
{
    minVoltage = 0, maxVoltage = 0;
    for (int i = 15; i < 24; ++i)
    {
        if ((ocr & (1 << i)) != 0)
        {
            if (minVoltage == 0) minVoltage = i - 15 + 27;
            maxVoltage = i - 15 + 28;
        }
    }
}

void SdCard::printOcr()
{
    uint32_t ocr = mSdio.shortResponse();
    printf("Card is ");
    if ((ocr & 0x80000000) == 0) printf("BUSY\n");
    else
    {
        printf("READY, type is ");
        if (ocr & 0x40000000) printf("HC/XC");
        else printf("SC");
        int min, max;
        voltageFromOcr(ocr, min, max);
        printf(" and supports %i.%i-%i.%iV.\n", min / 10, min % 10, max / 10, max % 10);
    }
}

uint32_t SdCard::getBits(uint8_t *field, int fieldSize, int highestBit, int lowestBit)
{
    int byteOffset = fieldSize - highestBit / 8 - 1;
    int bitOffset = highestBit % 8;
    int bitCount = highestBit - lowestBit + 1;
    if (bitCount > 32) assert("Bit count greater than 32 not allowed.");
    uint32_t ret = 0;
    while (bitCount > 0)
    {
        int shift = bitCount - bitOffset - 1;
        if (shift > 0) ret |= (field[byteOffset] << shift);
        else ret |= field[byteOffset] >> -shift;
        bitCount -= bitOffset + 1;
        ++byteOffset;
        bitOffset = 7;
    }
    ret &= 0xffffffff >> (31 - highestBit + lowestBit);
    return ret;
}

