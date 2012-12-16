/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <algorithm>
#include <cstring>

template<class T>
class CircularBuffer
{
public:
    CircularBuffer(unsigned int size)  :
        mSize(size),
        mBuffer(new T[size]),
        mWrite(mBuffer),
        mRead(mBuffer),
        mUsed(0)
    {
    }

    ~CircularBuffer()
    {
        delete mBuffer;
    }

    inline unsigned int used() { return mUsed; }
    inline unsigned int free() { return mSize - mUsed; }

    bool push(T c)
    {
        if (free() == 0) return false;
        ++mUsed;
        *mWrite = c;
        ++mWrite;
        align(mWrite);
        return true;
    }

    bool pop(T &c)
    {
        if (used() == 0) return false;
        --mUsed;
        c = *mRead;
        ++mRead;
        align(mRead);
        return true;
    }

    unsigned int write(const T *data, unsigned int len)
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

    unsigned int read(char *data, unsigned int len)
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


    unsigned int getContBuffer(const char*& data)
    {
        if (mUsed == 0) return 0;
        data = mRead;
        if (data < mWrite) return mWrite - data;
        return (mBuffer + mSize) - data;
    }

    unsigned int skip(unsigned int len)
    {
        len = std::min(len, used());
        mRead += len;
        mUsed -= len;
        if (mRead >= (mBuffer + mSize)) mRead -= mSize;
        return len;
    }

protected:
    unsigned int mSize;
    T* mBuffer;
    T* mWrite;
    T* mRead;
    volatile unsigned int mUsed;

    unsigned int writePart(const T* data, unsigned int len)
    {
        unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mWrite), std::min(len, free()));
        std::memcpy(mWrite, data, maxLen * sizeof(T));
        len -= maxLen;
        mWrite += maxLen;
        align(mWrite);
        return maxLen;
    }

    unsigned int readPart(T *data, unsigned int len)
    {
        unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mRead), std::min(len, used()));
        std::memcpy(data, mRead, maxLen * sizeof(T));
        len -= maxLen;
        mRead += maxLen;
        align(mRead);
        return maxLen;
    }

    inline void align(T*volatile& ptr) { if (ptr >= (mBuffer + mSize)) ptr = mBuffer; }

    friend int testCircularBuffer();

};


#endif // CIRCULARBUFFER_H
