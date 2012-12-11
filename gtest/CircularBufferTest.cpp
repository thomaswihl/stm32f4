#include "../CircularBuffer.h"
#include "../CircularBuffer.cpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define MAX_SIZE 1024

unsigned int mSize;
char mBuffer[MAX_SIZE];
char* mWrite;
char* mRead;
unsigned int mUsed;


CircularBuffer* mCircularBuffer;

void testCircularBuffer()
{
    memcpy(mBuffer, mCircularBuffer->mBuffer, mCircularBuffer->mSize);
    mSize = mCircularBuffer->mSize;
    mWrite = mCircularBuffer->mWrite;
    mRead = mCircularBuffer->mRead;
    mUsed = mCircularBuffer->mUsed;
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
