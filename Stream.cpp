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

#include "Stream.h"

template<typename T>
Stream<T>::Stream(System &system) :
    mSystem(system),
    mReadData(nullptr),
    mReadCount(0),
    mReadCallback(nullptr),
    mWriteData(nullptr),
    mWriteCount(0),
    mWriteCallback(nullptr),
    mReadBuffer(nullptr)
{

}

template<typename T>
void Stream<T>::readFifo(unsigned int size)
{
    if (mReadBuffer != nullptr)
    {
        delete mReadBuffer;
        mReadBuffer = nullptr;
    }
    if (size > 0)
    {
        mReadBuffer = new CircularBuffer<T>(size);
        triggerRead();
    }
}

template<typename T>
bool Stream<T>::readPrepare(T *data, unsigned int count)
{
    if (mReadCount != 0 || count == 0) return false;
    if (readFromBuffer(data, count)) return false;
    mReadCount = count;
    mReadData = data;
    return true;
}

template<typename T>
bool Stream<T>::readPrepare(T *data, unsigned int count, System::Event *callback)
{
    if (mReadCount != 0 || count == 0) return false;
    mReadCallback = callback;
    return readPrepare(data, count);
}

template<typename T>
bool Stream<T>::read(T data)
{
    if (mReadBuffer != nullptr)
    {
        mReadBuffer->push(data);
        if (mReadCount != 0 && mReadData != nullptr) readFromBuffer(mReadData, mReadCount);
        return true;
    }
    else if (mReadCount != 0 && mReadData != nullptr)
    {
        *mReadData++ = data;
        --mReadCount;
        if (mReadCount == 0)
        {
            readFinished(true);
            return false;
        }
    }
    return true;
}

template<typename T>
void Stream<T>::readFinished(bool success)
{
    mReadCount = 0;
    mReadData = 0;
    if (mReadCallback != nullptr)
    {
        mReadCallback->setResult(success);
        System::postEvent(mReadCallback);
        mReadCallback = nullptr;
    }
}

template<typename T>
T *Stream<T>::readData()
{
    if (mReadBuffer != nullptr)
    {
        return mReadBuffer->writePointer();
    }
    return mReadData;
}

template<typename T>
unsigned int Stream<T>::readCount()
{
    if (mReadBuffer != nullptr) return 1;
    return mReadCount;
}

template<typename T>
bool Stream<T>::writePrepare(const T *data, unsigned int count)
{
    if (mWriteCount > 0 || count == 0) return false;
    mWriteData = data;
    mWriteCount = count;
    return true;
}

template<typename T>
bool Stream<T>::writePrepare(const T *data, unsigned int count, System::Event *callback)
{
    if (mWriteCount > 0 || count == 0) return false;
    mWriteCallback = callback;
    return writePrepare(data, count);
}

template<typename T>
bool Stream<T>::write(T &data)
{
    if (mWriteCount > 0 && mWriteData != nullptr)
    {
        --mWriteCount;
        data = *mWriteData++;
        return true;
    }
    if (mWriteCount < 0)
    {
        printf("\nWC<0: %i, %c %c %c\n", mWriteCount, *(mWriteData - 2), *(mWriteData - 1), *(mWriteData));
    }
    writeFinished(true);
    return false;
}

template<typename T>
void Stream<T>::writeFinished(bool success)
{
    mWriteCount = 0;
    mWriteData = 0;
    if (mWriteCallback != nullptr)
    {
        mWriteCallback->setResult(success);
        System::postEvent(mWriteCallback);
        mWriteCallback = nullptr;
    }
}

template<typename T>
const T *Stream<T>::writeData()
{
    return mWriteData;
}

template<typename T>
unsigned int Stream<T>::writeCount()
{
    return mWriteCount;
}

template<typename T>
bool Stream<T>::readFromBuffer(T*& data, unsigned int& count)
{
    if (mReadBuffer != nullptr)
    {
        int len = mReadBuffer->read(data, count);
        count -= len;
        data += len;
        if (count == 0)
        {
            readFinished(true);
            return true;
        }
    }
    return false;
}

template class Stream<char>;
template class Stream<uint16_t>;


template<typename T>
void BufferedStream<T>::read(T *data, unsigned int count)
{
    while (mReadBuffer.used() == 0)
    {
        __asm("wfi");
    }

    mReadBuffer.read(data, count);
}

template<typename T>
void BufferedStream<T>::read(T *data, unsigned int count, System::Event *callback)
{
}

template<typename T>
void BufferedStream<T>::write(const T *data, unsigned int count)
{
}

template<typename T>
void BufferedStream<T>::write(const T *data, unsigned int count, System::Event *callback)
{
}
