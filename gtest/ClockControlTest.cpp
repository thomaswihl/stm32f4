#include "../ClockControl.h"
#include "../ClockControl.cpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define SIZE_OF_RCC 0x88

static const uint32_t externalClock[] =
{
    8000000,
    25000000,
    64000000,
    2250000,
};
static const uint32_t pllClock[] =
{
    168000000,
     96000000,
    216000000,
     50000000,
};
static const bool retResult[ARRAY_SIZE(externalClock)][ARRAY_SIZE(pllClock)] =
{
    { true, true, true, false },
    { true, true, true, false },
    { true, false, true, false },
    { true, false, true, false },
};

void testClockControl()
{
    static const uint32_t mulResult[ARRAY_SIZE(externalClock)][ARRAY_SIZE(pllClock)] =
    {
        { 210, 192, 216, 192 },
        { 336, 192, 432, 192 },
        { 210, 192, 216, 192 },
        { 298, 192, 384, 192 },
    };
    static const uint32_t divResult[ARRAY_SIZE(externalClock)][ARRAY_SIZE(pllClock)] =
    {
        { 5, 8, 4, 63 },
        { 25, 25, 25, 63 },
        { 40, 63, 32, 63 },
        { 2, 63, 2, 63 },
    };
    ClockControl::RCC data;
    ASSERT_EQ(sizeof(data), SIZE_OF_RCC) << "Wrong structure size, compiler problem";
    for (unsigned int i = 0; i < ARRAY_SIZE(externalClock); ++i)
    {
        std::memset(&data, 0, sizeof(data));
        unsigned long base = reinterpret_cast<unsigned long>(&data);
        ClockControl cc(base, externalClock[i]);
        for (unsigned int j = 0; j < ARRAY_SIZE(pllClock); ++j)
        {
            uint32_t div, mul;
            bool ret = cc.getPllConfig(pllClock[j] * 2, div, mul);
            EXPECT_EQ(retResult[i][j], ret);
            EXPECT_EQ(mulResult[i][j], mul);
            EXPECT_EQ(divResult[i][j], div);
            if (ret)
            {
                EXPECT_GE(externalClock[i] / div, 1000000) << "VCO input should be between 1 and 2 MHz";
                EXPECT_LE(externalClock[i] / div, 2000000) << "VCO input should be between 1 and 2 MHz";
                EXPECT_GE(externalClock[i] / div * mul, 192000000) << "VCO onput should be between 192 and 432 MHz";
                EXPECT_LE(externalClock[i] / div * mul, 432000000) << "VCO onput should be between 192 and 432 MHz";
            }
        }
    }
}

TEST(ClockControl, getPllConfig)
{
    testClockControl();
}

TEST(ClockControl, systemClock)
{
    uint32_t* data = new uint32_t[SIZE_OF_RCC / 4];
    unsigned long base = reinterpret_cast<unsigned long>(data);
    for (unsigned int i = 0; i < ARRAY_SIZE(externalClock); ++i)
    {
        for (unsigned int j = 0; j < ARRAY_SIZE(pllClock); ++j)
        {
            std::memset(data, 0, SIZE_OF_RCC);
            data[0] |= 0x02020000;  // HSERDY, PLLRDY
            data[2] |= 0x00000008;  // SWS = 2;

            ClockControl cc(base, externalClock[i]);
            bool ret = cc.setSystemClock(pllClock[j]);
            EXPECT_EQ(retResult[i][j], ret);
            if (ret) EXPECT_GE(pllClock[j], cc.systemClock());
        }
    }
    delete[] data;
}

TEST(ClockControl, reset)
{
    uint32_t* data = new uint32_t[SIZE_OF_RCC / 4];
    unsigned long base = reinterpret_cast<unsigned long>(data);
    std::memset(data, 0, SIZE_OF_RCC);
    data[0] |= 0x02020000;  // HSERDY, PLLRDY
    data[2] |= 0x00000008;  // SWS = 2;

    ClockControl cc(base, externalClock[0]);
    bool ret = cc.setSystemClock(pllClock[0]);
    EXPECT_EQ(retResult[0][0], ret);
    if (ret) EXPECT_GE(pllClock[0], cc.systemClock());
    cc.reset();
    EXPECT_EQ(0x00000081, data[0]);
    EXPECT_EQ(0x24003010, data[1]);
    EXPECT_EQ(0x00000000, data[2]);
    EXPECT_EQ(0x00000000, data[3]);
    EXPECT_EQ(0x00000000, data[4]);
    EXPECT_EQ(0x00000000, data[5]);
    EXPECT_EQ(0x00000000, data[6]);
    EXPECT_EQ(0x00000000, data[7]);
    EXPECT_EQ(0x00000000, data[8]);
    EXPECT_EQ(0x00000000, data[9]);
    EXPECT_EQ(0x00000000, data[10]);
    EXPECT_EQ(0x00000000, data[11]);
    EXPECT_EQ(0x00100000, data[12]);
    EXPECT_EQ(0x00000000, data[13]);
    EXPECT_EQ(0x00000000, data[14]);
    EXPECT_EQ(0x00000000, data[15]);
    EXPECT_EQ(0x00000000, data[16]);
    EXPECT_EQ(0x00000000, data[17]);
    EXPECT_EQ(0x00000000, data[18]);
    EXPECT_EQ(0x00000000, data[19]);
    EXPECT_EQ(0x7e6791ff, data[20]);
    EXPECT_EQ(0x000000f1, data[21]);
    EXPECT_EQ(0x00000001, data[22]);
    EXPECT_EQ(0x00000000, data[23]);
    EXPECT_EQ(0x36fec9ff, data[24]);
    EXPECT_EQ(0x00075f33, data[25]);
    EXPECT_EQ(0x00000000, data[26]);
    EXPECT_EQ(0x00000000, data[27]);
    EXPECT_EQ(0x00000000, data[28]);
    EXPECT_EQ(0x0e000000, data[29]);
    EXPECT_EQ(0x00000000, data[30]);
    EXPECT_EQ(0x00000000, data[31]);
    EXPECT_EQ(0x00000000, data[32]);
    EXPECT_EQ(0x20003000, data[33]);

    delete[] data;
}
