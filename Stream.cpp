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
Stream<T>::Stream() :
    mReadData(nullptr),
    mReadCount(0),
    mReadCompleteEvent(nullptr),
    mWriteData(nullptr),
    mWriteCount(0),
    mWriteCompleteEvent(nullptr),
    mReadFifo(nullptr),
    mWriteFifo(nullptr)
{
}

template<typename T>
Stream<T>::~Stream()
{
}

template<typename T>
bool Stream<T>::read(T *data, unsigned int count)
{
    if (!readProlog(data, count)) return false;
    if (mReadCount != 0)
    {
        readSync();
        readEpilog();
    }
    return true;
}

template<typename T>
bool Stream<T>::read(T *data, unsigned int count, System::Event *completeEvent)
{
    mReadCompleteEvent = completeEvent;
    if (!readProlog(data, count))
    {
        mReadCompleteEvent = nullptr;
        return false;
    }
    if (mReadCount != 0)
    {
        readTrigger();
    }
    return true;
}

template<typename T>
bool Stream<T>::write(const T *data, unsigned int count)
{
    if (!writeProlog(data, count)) return false;
    if (mWriteCount != 0)
    {
        writeSync();
        writeEpilog();
    }
    return true;
}

template<typename T>
bool Stream<T>::write(const T *data, unsigned int count, System::Event *completeEvent)
{
    mWriteCompleteEvent = completeEvent;
    if (!writeProlog(data, count))
    {
        mWriteCompleteEvent = nullptr;
        return false;
    }
    if (mWriteCount != 0)
    {
        writeTrigger();
    }
    return true;
}

template<typename T>
void Stream<T>::readFifo(unsigned int size)
{
    if (mReadFifo != nullptr)
    {
        delete mReadFifo;
        mReadFifo = nullptr;
    }
    if (size > 0)
    {
        mReadFifo = new CircularBuffer<T>(size);
        readTrigger();
    }
}

template<typename T>
void Stream<T>::writeFifo(unsigned int size)
{
    if (mWriteFifo != nullptr)
    {
        delete mWriteFifo;
        mWriteFifo = nullptr;
    }
    if (size > 0)
    {
        mWriteFifo = new CircularBuffer<T>(size);
    }
}

template<typename T>
void Stream<T>::readDmaComplete(unsigned int count)
{
    if (mReadFifo != nullptr)
    {

    }
    else
    {
        mReadData += count;
        mReadCount -= count;
    }
    if (mReadCount == 0) readEpilog();
}

template<typename T>
bool Stream<T>::read(T data)
{
    if (mReadFifo != nullptr)
    {
        mReadFifo->push(data);
        readFromFifo(mReadData, mReadCount);
        if (mReadCount == 0 && mReadData != nullptr) readEpilog();
        return true;
    }
    else if (mReadCount != 0)
    {
        *mReadData++ = data;
        --mReadCount;
        if (mReadCount == 0)
        {
            readEpilog();
            return false;
        }
    }
    return true;
}

template<typename T>
void Stream<T>::readDmaBuffer(T *&data, unsigned int &count)
{
    if (mReadFifo != nullptr)
    {
        data = mReadFifo->writePointer();
        if (mReadFifo->free() > 0) count = 1;
        else count = 0;
    }
    else
    {
        data = mReadData;
        count = mReadCount;
    }
}

template<typename T>
void Stream<T>::writeDmaComplete(unsigned int count)
{
    if (mWriteFifo != nullptr)
    {
        mWriteFifo->skip(count);
        if (mWriteFifo->used() != 0) writeTrigger();
    }
    else
    {
        mWriteData += count;
        mWriteCount -= count;
        if (mWriteCount == 0) writeEpilog();
        else writeTrigger();
    }
}

template<typename T>
bool Stream<T>::write(T &data)
{
    if (mWriteFifo != nullptr)
    {
        if (mWriteFifo->pop(data)) return true;
    }
    else if (mWriteCount != 0)
    {
        data = *mWriteData++;
        --mWriteCount;
        return true;
    }
    writeEpilog();
    return false;
}

template<typename T>
void Stream<T>::writeDmaBuffer(const T *&data, unsigned int &count)
{
    if (mWriteFifo != nullptr)
    {
        count = mWriteFifo->getContBuffer(data);
    }
    else
    {
        data = mWriteData;
        count = mWriteCount;
    }
}

template<typename T>
bool Stream<T>::readProlog(T *data, unsigned int count)
{
    if (mReadData != nullptr) return false;
    readFromFifo(data, count);
    if (count == 0)
    {
        readEpilog();
        return true;
    }
    mReadCount = count;
    mReadData = data;
    readPrepare();
    return true;
}

template<typename T>
void Stream<T>::readEpilog()
{
    if (mReadData != nullptr)
    {
        readDone();
        mReadCount = 0;
        mReadData = nullptr;
    }
    if (mReadCompleteEvent != nullptr)
    {
        System::postEvent(mReadCompleteEvent);
        mReadCompleteEvent = nullptr;
    }
}

template<typename T>
void Stream<T>::readFromFifo(T *&data, unsigned int &count)
{
    if (mReadFifo != nullptr && data != nullptr && count != 0)
    {
        int len = mReadFifo->read(data, count);
        count -= len;
        data += len;
    }
}

template<typename T>
bool Stream<T>::writeProlog(const T *data, unsigned int count)
{
    if (mWriteData != nullptr) return false;
    if (mWriteFifo != nullptr)
    {
        while (count != 0) writeToFifo(data, count);
    }
    if (count == 0)
    {
        writeEpilog();
        return true;
    }
    mWriteCount = count;
    mWriteData = data;
    writePrepare();
    return true;
}

template<typename T>
void Stream<T>::writeEpilog()
{
    if (mWriteData != nullptr)
    {
        writeDone();
        mWriteCount = 0;
        mWriteData = nullptr;
    }
    if (mWriteCompleteEvent != nullptr)
    {
        System::postEvent(mWriteCompleteEvent);
        mWriteCompleteEvent = nullptr;
    }
}

template<typename T>
void Stream<T>::writeToFifo(const T *&data, unsigned int &count)
{
    if (mWriteFifo != nullptr && data != nullptr && count != 0)
    {
        bool fifoEmpty = mWriteFifo->used() == 0;
        int len = mWriteFifo->write(data, count);
        count -= len;
        data += len;
        if (fifoEmpty) writeTrigger();
    }
}

template class Stream<char>;
template class Stream<uint16_t>;

