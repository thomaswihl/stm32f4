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
    mReadCompleteEvent(nullptr),
    mWriteData(nullptr),
    mWriteCount(0),
    mWriteCompleteEvent(nullptr),
    mReadFifo(nullptr),
    mWriteFifo(nullptr)
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
        writeTrigger();
    }
}

template<typename T>
void Stream<T>::readSuccess(bool success)
{
    if (mReadCompleteEvent != nullptr) mReadCompleteEvent->setResult(success);
    readEpilog();
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
            readSuccess(true);
            return false;
        }
    }
    return true;
}

template<typename T>
T *Stream<T>::readData()
{
    if (mReadFifo != nullptr)
    {
        return mReadFifo->writePointer();
    }
    return mReadData;
}

template<typename T>
unsigned int Stream<T>::readCount()
{
    if (mReadFifo != nullptr) return 1;
    return mReadCount;
}

template<typename T>
void Stream<T>::writeSuccess(bool success)
{
    if (mWriteCompleteEvent != nullptr) mWriteCompleteEvent->setResult(success);
    writeEpilog();
}

template<typename T>
bool Stream<T>::write(T &data)
{
    if (mWriteFifo != nullptr)
    {
        mWriteFifo->push(data);
        writeFromFifo();
        if (mWriteCount == 0) writeEpilog();
        return true;
    }
    else if (mWriteCount != 0)
    {
        data = *mWriteData++;
        --mWriteCount;
        return true;
    }
    writeSuccess(true);
    return false;
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
bool Stream<T>::readProlog(T *data, unsigned int count)
{
    if (mReadData != nullptr) return false;
    readFromFifo(data, count);
    if (count == 0)
    {
        readSuccess(true);
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
    if (data != nullptr && count != 0)
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
    mWriteCount = count;
    mWriteData = data;
    writeFromFifo();
    if (mWriteCount != 0) writePrepare();
    else writeEpilog();
    return true;
}

template<typename T>
void Stream<T>::writeEpilog()
{
    writeDone();
    mWriteCount = 0;
    mWriteData = nullptr;
    if (mWriteCompleteEvent != nullptr)
    {
        System::postEvent(mWriteCompleteEvent);
        mWriteCompleteEvent = nullptr;
    }
}

template<typename T>
void Stream<T>::writeFromFifo()
{
    if (mWriteFifo != nullptr && mWriteCount != 0)
    {
        //int len = mWriteFifo->read(mWriteData, mWriteCount);
        //mWriteCount -= len;
        //mWriteData += len;
    }
}

template class Stream<char>;
template class Stream<uint16_t>;

