#include "CircularBuffer.h"
#include <algorithm>
#include <cstring>

CircularBuffer::CircularBuffer(unsigned int size) :
    mSize(size),
    mBuffer(new char[size]),
    mWrite(mBuffer),
    mRead(mBuffer),
    mUsed(0)
{
}

CircularBuffer::~CircularBuffer()
{
    delete mBuffer;
}

bool CircularBuffer::push(char c)
{
    if (free() == 0) return false;
    ++mUsed;
    *mWrite++ = c;
    align(mWrite);
    return true;
}

bool CircularBuffer::pop(char& c)
{
    if (used() == 0) return false;
    --mUsed;
    c = *mRead++;
    align(mRead);
    return true;
}

unsigned int CircularBuffer::write(const char *data, unsigned int len)
{
    unsigned int totalLen = 0;
    while (len > 0 && free() != 0)
    {
        unsigned int partLen = writePart(data, len);
        data += partLen;
        len -= partLen;
        mUsed += partLen;
        totalLen += partLen;
    }
    return totalLen;
}

unsigned int CircularBuffer::read(char *data, unsigned int len)
{
    unsigned int totalLen = 0;
    while (len > 0 && used() != 0)
    {
        unsigned int partLen = readPart(data, len);
        data += partLen;
        len -= partLen;
        mUsed -= partLen;
        totalLen += partLen;
    }
    return totalLen;
}

unsigned int CircularBuffer::getContBuffer(char *&data)
{
    data = mRead;
    if (data < mWrite) return mWrite - data;
    return (mBuffer + mSize) - data;
}

unsigned int CircularBuffer::skip(unsigned int len)
{
    len = std::min(len, used());
    mRead += len;
    if (mRead >= (mBuffer + mSize)) mRead -= mSize;
    return len;
}

unsigned int CircularBuffer::writePart(const char *data, unsigned int len)
{
    unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mWrite), std::min(len, free()));
    std::memcpy(mWrite, data, maxLen);
    len -= maxLen;
    mWrite += maxLen;
    align(mWrite);
    return maxLen;
}

unsigned int CircularBuffer::readPart(char *data, unsigned int len)
{
    unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mRead), std::min(len, used()));
    std::memcpy(data, mRead, maxLen);
    len -= maxLen;
    mRead += maxLen;
    align(mRead);
    return maxLen;
}

