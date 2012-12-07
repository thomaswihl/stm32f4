#include "../ClockControl.h"
#include "../ClockControl.cpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
void testClockControl()
{
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
    static const bool retResult[ARRAY_SIZE(externalClock)][ARRAY_SIZE(pllClock)] =
    {
        { true, true, true, false },
        { true, true, true, false },
        { true, false, true, false },
        { true, false, true, false },
    };
    ClockControl::RCC data;
    ASSERT_EQ(sizeof(data), 0x88) << "Wrong structure size, compiler problem";
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

