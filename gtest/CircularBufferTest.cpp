#include "../CircularBuffer.h"
#include "../CircularBuffer.cpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <random>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define MAX_SIZE 1024
#define MODULO 251

unsigned int mSize;
unsigned char mBuffer[MAX_SIZE];
unsigned int mWrite;
unsigned int mRead;
unsigned int mUsed;


CircularBuffer* mCircularBuffer;

int testCircularBuffer()
{
    memcpy(mBuffer, mCircularBuffer->mBuffer, mCircularBuffer->mSize);
    mSize = mCircularBuffer->mSize;
    mWrite = mCircularBuffer->mWrite - mCircularBuffer->mBuffer;
    mRead = mCircularBuffer->mRead - mCircularBuffer->mBuffer;
    mUsed = mCircularBuffer->mUsed;
    printf("mSize = %u, mWrite = %08x, mRead = %08x, mUsed = %u", mSize, mWrite, mRead, mUsed);
//    for (unsigned int i = 0; i < mSize; ++i)
//    {
//        if ((i % 16) == 0) printf("\n%08x:", i);
//        printf(" %02x", mBuffer[i]);
//    }
//    printf("\n");
    return 0;
}

TEST(CircularBuffer, push)
{
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    for (int i = 0; i < MAX_SIZE; ++i)
    {
        EXPECT_EQ(i, mCircularBuffer->used());
        EXPECT_EQ(MAX_SIZE - i, mCircularBuffer->free());
        EXPECT_TRUE(mCircularBuffer->push(i)) << "Could not push enough elements";
    }
    EXPECT_EQ(MAX_SIZE, mCircularBuffer->used());
    EXPECT_EQ(0, mCircularBuffer->free());
    EXPECT_FALSE(mCircularBuffer->push(0)) << "Could push too much elements";
}

TEST(CircularBuffer, pushPopRandom)
{
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free);
        dist.param(param);
        int pushCount = dist(generator);
        for (int pc = 0; pc < pushCount; ++pc)
        {
            EXPECT_TRUE(mCircularBuffer->push(in % MODULO)) << "Could not push enough elements";
            ++in;
        }
        free -= pushCount;
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());

        param = std::uniform_int_distribution<int>::param_type(1, size - free);
        dist.param(param);
        int popCount = dist(generator);
        for (int pc = 0; pc < popCount; ++pc)
        {
            char c;
            EXPECT_TRUE(mCircularBuffer->pop(c)) << "Could not pop enough elements";
            ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(c));
            ++out;
        }
        free += popCount;
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
    }
}

TEST(CircularBuffer, readWriteRandom)
{
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    char buf[size];
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free);
        dist.param(param);
        int pushCount = dist(generator);
        for (int pc = 0; pc < pushCount; ++pc)
        {
            buf[pc] = in % MODULO;
            ++in;
        }
        EXPECT_EQ(pushCount, mCircularBuffer->write(buf, pushCount));
        free -= pushCount;
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());

        param = std::uniform_int_distribution<int>::param_type(1, size - free);
        dist.param(param);
        int popCount = dist(generator);
        EXPECT_EQ(popCount, mCircularBuffer->read(buf, popCount));
        for (int pc = 0; pc < popCount; ++pc)
        {
            ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(buf[pc]));
            ++out;
        }
        free += popCount;
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
    }
}

TEST(CircularBuffer, bufferFullReadWrite)
{
    static unsigned int extra = 10;
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    char buf[size + extra];
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free + extra);
        dist.param(param);
        int pushCount = dist(generator);
        int pushed = 0;
        for (int pc = 0; pc < pushCount; ++pc)
        {
            buf[pc] = in % MODULO;
            if (free > 0)
            {
                ++in;
                --free;
                ++pushed;
            }
        }
        EXPECT_EQ(pushed, mCircularBuffer->write(buf, pushCount));
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
        if (free == 0)
        {
            param = std::uniform_int_distribution<int>::param_type(1, size - free);
            dist.param(param);
            int popCount = dist(generator);
            EXPECT_EQ(popCount, mCircularBuffer->read(buf, popCount));
            for (int pc = 0; pc < popCount; ++pc)
            {
                ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(buf[pc]));
                ++out;
            }
            free += popCount;
            EXPECT_EQ(free, mCircularBuffer->free());
            EXPECT_EQ(size - free, mCircularBuffer->used());
        }
    }
}

TEST(CircularBuffer, bufferFullPushPop)
{
    static unsigned int extra = 100;
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free + extra);
        dist.param(param);
        int pushCount = dist(generator);
        int pushed = 0;
        for (int pc = 0; pc < pushCount; ++pc)
        {
            if (free > 0)
            {
                EXPECT_TRUE(mCircularBuffer->push(in % MODULO)) << "Could not push enough elements";
                ++in;
                --free;
                ++pushed;
            }
            else
            {
                EXPECT_FALSE(mCircularBuffer->push(in % MODULO)) << "Could push too much elements";
            }
        }
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
        if (free == 0)
        {
            param = std::uniform_int_distribution<int>::param_type(1, size - free);
            dist.param(param);
            int popCount = dist(generator);
            for (int pc = 0; pc < popCount; ++pc)
            {
                char c;
                EXPECT_TRUE(mCircularBuffer->pop(c)) << "Could not pop enough elements";
                ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(c));
                ++out;
            }
            free += popCount;
            EXPECT_EQ(free, mCircularBuffer->free());
            EXPECT_EQ(size - free, mCircularBuffer->used());
        }
    }
}


TEST(CircularBuffer, bufferEmptyReadWrite)
{
    static unsigned int extra = 100;
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    char buf[size + extra];
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free);
        if (free == size)
        {
            dist.param(param);
            int pushCount = dist(generator);
            for (int pc = 0; pc < pushCount; ++pc)
            {
                buf[pc] = in % MODULO;
                ++in;
                --free;
            }
            EXPECT_EQ(pushCount, mCircularBuffer->write(buf, pushCount));
            EXPECT_EQ(free, mCircularBuffer->free());
            EXPECT_EQ(size - free, mCircularBuffer->used());
        }
        param = std::uniform_int_distribution<int>::param_type(1, size - free + extra);
        dist.param(param);
        unsigned int popCount = dist(generator);
        unsigned int realPopCount = std::min(popCount, size - free);
        EXPECT_EQ(realPopCount, mCircularBuffer->read(buf, popCount));
        for (unsigned int pc = 0; pc < realPopCount; ++pc)
        {
            ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(buf[pc]));
            ++out;
            ++free;
        }
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
    }
}

TEST(CircularBuffer, bufferEmptyPushPop)
{
    static unsigned int extra = 100;
    mCircularBuffer = new CircularBuffer(MAX_SIZE);
    unsigned int in = 0;
    unsigned int out = 0;
    std::default_random_engine generator(7);
    unsigned int size = mCircularBuffer->free();
    unsigned int free = size;
    std::uniform_int_distribution<int> dist(1, free);
    char buf[size + extra];
    for (int i = 0; i < 1000; ++i)
    {
        std::uniform_int_distribution<int>::param_type param(1, free);
        if (free == size)
        {
            dist.param(param);
            int pushCount = dist(generator);
            for (int pc = 0; pc < pushCount; ++pc)
            {
                EXPECT_TRUE(mCircularBuffer->push(in % MODULO));
                ++in;
                --free;
            }
            EXPECT_EQ(free, mCircularBuffer->free());
            EXPECT_EQ(size - free, mCircularBuffer->used());
        }
        param = std::uniform_int_distribution<int>::param_type(1, size - free + extra);
        dist.param(param);
        unsigned int popCount = dist(generator);
        unsigned int realPopCount = std::min(popCount, size - free);
        for (unsigned int pc = 0; pc < realPopCount; ++pc)
        {
            char c;
            EXPECT_TRUE(mCircularBuffer->pop(c));
            ASSERT_EQ(static_cast<unsigned char>(out % MODULO), static_cast<unsigned char>(c));
            ++out;
            ++free;
        }
        for (unsigned int pc = realPopCount; pc < popCount; ++pc)
        {
            char c;
            EXPECT_FALSE(mCircularBuffer->pop(c));
        }
        EXPECT_EQ(free, mCircularBuffer->free());
        EXPECT_EQ(size - free, mCircularBuffer->used());
    }
}
