#include "../ClockControl.h"
#include "../ClockControl.cpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
int testClockControl()
{
    int failCounter = 0;
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
    if (sizeof(data) != 0x88)
    {
        printf("Size of RCC is %lu instead of %i.\n", sizeof(data), 0x88);
        return false;
    }
    printf("%7s|%7s|%14s|%12s|%12s|%14s|%20s|%12s|%10s\n", "ext", "pll", "return", "div", "mul", "vco in [MHz]", "vco out [MHz]", "delta [Hz]", "verdict");
    for (unsigned int i = 0; i < ARRAY_SIZE(externalClock); ++i)
    {
        std::memset(&data, 0, sizeof(data));
        unsigned long base = reinterpret_cast<unsigned long>(&data);
        ClockControl cc(base, externalClock[i]);
        for (unsigned int j = 0; j < ARRAY_SIZE(pllClock); ++j)
        {
            uint32_t div, mul;
            bool ret = cc.getPllConfig(pllClock[j] * 2, div, mul);
            bool test = (ret == retResult[i][j] && mul == mulResult[i][j] && div == divResult[i][j]);
            if (ret) test = test && (externalClock[i] / div) >= 1000000 && (externalClock[i] / div) <= 2000000;
            if (ret) test = test && (externalClock[i] / div * mul) >= 192000000 && (externalClock[i] / div * mul) <= 432000000;
            printf("%7.3f|%7.3f|%5s %2s %5s|%4u %2s %4u|%4u %2s %4u|",
                   externalClock[i] / 1000000.0,
                   pllClock[j] / 1000000.0,
                   ret ? "true" : "false", ret == retResult[i][j] ? "==" : "!=", retResult[i][j] ? "true" : "false",
                    div, div == divResult[i][j] ? "==" : "!=", divResult[i][j],
                    mul, mul == mulResult[i][j] ? "==" : "!=", mulResult[i][j]);
            if (ret)
            {
                printf("1 <= %1.2f <= 2|192 <= %6.2f <= 432|%12i|",
                   externalClock[i] / div / 1000000.0,
                   externalClock[i] / div * mul / 1000000.0,
                   externalClock[i] / div * mul - pllClock[j] * 2);
            }
            else
            {
                printf("%14s|%20s|%12s|", "", "", "");
            }
            printf("%10s\n", test ? "PASSED" : "FAILED");
            if (!test) ++failCounter;
        }
    }
    return failCounter;
}

TEST(ClockControl, getPllConfig)
{
    testClockControl();
}

