#include "CircularBuffer.h"
#include <algorithm>
#include <cstring>

CircularBuffer::CircularBuffer(unsigned int size) :
    mSize(size),
    mBuffer(new char[size]),
    mNextWrite(mBuffer),
    mLastRead(mBuffer + size - 1)
{
}

CircularBuffer::~CircularBuffer()
{
    delete mBuffer;
}

unsigned int CircularBuffer::size()
{
    int diff = mNextWrite - (mLastRead + 1);
    if (diff < 0) diff += mSize;
    return static_cast<unsigned int>(diff);
}

unsigned int CircularBuffer::free()
{
    return mSize - size();
}

bool CircularBuffer::push(char c)
{
    if (mLastRead == mNextWrite) return false;
    *mNextWrite++ = c;
    if (mNextWrite >= (mBuffer + mSize)) mNextWrite = mBuffer;
    return true;
}

char CircularBuffer::pop()
{
    ++mLastRead;
    if (mLastRead >= (mBuffer + mSize)) mLastRead = mBuffer;
    return *mLastRead;
}

unsigned int CircularBuffer::append(const char *data, unsigned int len)
{
    unsigned int totalAppended = 0;
    while (len > 0 && mNextWrite != mLastRead)
    {
        unsigned int appended = appendPart(data, len);
        data += appended;
        len -= appended;
        totalAppended += appended;
    }
    return totalAppended;
}

unsigned int CircularBuffer::getContBuffer(char *&data)
{
    data = mLastRead + 1;
    if (data >= (mBuffer + mSize)) data = mBuffer;
    if (data < mNextWrite) return mNextWrite - data;
    return (mBuffer + mSize) - data;
}

unsigned int CircularBuffer::appendPart(const char *data, unsigned int len)
{
    unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mNextWrite), std::min(len, free()));
    std::memcpy(mNextWrite, data, maxLen);
    len -= maxLen;
    mNextWrite += maxLen;
    if (mNextWrite >= (mBuffer + mSize)) mNextWrite = mBuffer;
    return maxLen;
}

