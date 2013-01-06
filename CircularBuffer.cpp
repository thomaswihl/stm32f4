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

#include "System.h"
#include "CircularBuffer.h"

template<typename T>
CircularBuffer<T>::CircularBuffer(unsigned int size)  :
    mSize(size),
    mBuffer(new T[size]),
    mWrite(mBuffer),
    mRead(mBuffer),
    mUsed(0)
{
}

template<typename T>
CircularBuffer<T>::~CircularBuffer()
{
    delete mBuffer;
}

template<typename T>
bool CircularBuffer<T>::push(T elem)
{
    if (free() == 0) return false;
    ++mUsed;
    *mWrite = elem;
    ++mWrite;
    align(mWrite);
    return true;
}

template<typename T>
bool CircularBuffer<T>::pop(T &elem)
{
    if (used() == 0) return false;
    --mUsed;
    elem = *mRead;
    ++mRead;
    align(mRead);
    return true;
}

template<typename T>
unsigned int CircularBuffer<T>::write(const T* data, unsigned int len)
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

template<typename T>
unsigned int CircularBuffer<T>::read(T* data, unsigned int len)
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

template<typename T>
T CircularBuffer<T>::operator [](int index)
{
    T* volatile p;
    if (index < 0)
    {
        p = mWrite + index;
        while (p < mBuffer) p += mSize;
    }
    else
    {
        p = mRead + index;
    }
    align(p);
    return *p;
}

template<typename T>
T *CircularBuffer<T>::writePointer()
{
    return mWrite;
}

template<typename T>
T *CircularBuffer<T>::readPointer()
{
    return mRead;
}

template<typename T>
T *CircularBuffer<T>::bufferPointer()
{
    return mBuffer;
}

template<typename T>
unsigned int CircularBuffer<T>::getContBuffer(const T *&data)
{
    if (mUsed == 0) return 0;
    data = mRead;
    if (data < mWrite) return mWrite - data;
    return (mBuffer + mSize) - data;
}

template<typename T>
unsigned int CircularBuffer<T>::skip(unsigned int len)
{
    len = std::min(len, used());
    mRead += len;
    mUsed -= len;
    if (mRead >= (mBuffer + mSize)) mRead -= mSize;
    return len;
}

template<typename T>
unsigned int CircularBuffer<T>::writePart(const T *data, unsigned int len)
{
    unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mWrite), std::min(len, free()));
    std::memcpy(mWrite, data, maxLen * sizeof(T));
    len -= maxLen;
    mWrite += maxLen;
    align(mWrite);
    return maxLen;
}

template<typename T>
unsigned int CircularBuffer<T>::readPart(T *data, unsigned int len)
{
    unsigned int maxLen = std::min(static_cast<unsigned int>((mBuffer + mSize) - mRead), std::min(len, used()));
    std::memcpy(data, mRead, maxLen * sizeof(T));
    len -= maxLen;
    mRead += maxLen;
    align(mRead);
    return maxLen;
}

template class CircularBuffer<char>;
template class CircularBuffer<uint16_t>;
template class CircularBuffer<System::Event*>;
template class CircularBuffer<char*>;
