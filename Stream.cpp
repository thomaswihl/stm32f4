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
    mWriteCallback(nullptr)
{

}

template<typename T>
bool Stream<T>::readPrepare(T *data, unsigned int count)
{
    if (mReadCount != 0 || count == 0) return false;
    mReadData = data;
    mReadCount = count;
    return true;
}

template<typename T>
bool Stream<T>::readPrepare(T *data, unsigned int count, System::Event *callback)
{
    if (mReadCount != 0 || count == 0) return false;
    readPrepare(data, count);
    mReadCallback = callback;
    return true;
}

template<typename T>
bool Stream<T>::read(T data)
{
    if (mReadCount != 0 && mReadData != 0)
    {
        *mReadData++ = data;
        --mReadCount;
        return true;
    }
    return false;
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
bool Stream<T>::writePrepare(const T *data, unsigned int count)
{
    if (mWriteCount != 0 || count == 0) return false;
    mWriteData = data;
    mWriteCount = count;
    return true;
}

template<typename T>
bool Stream<T>::writePrepare(const T *data, unsigned int count, System::Event *callback)
{
    if (mWriteCount != 0 || count == 0) return false;
    writePrepare(data, count);
    mWriteCallback = callback;
    return true;
}

template<typename T>
bool Stream<T>::write(T &data)
{
    if (mWriteCount != 0 && mWriteData != 0)
    {
        data = *mWriteData++;
        --mWriteCount;
        return true;
    }
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


//do
//{
//    unsigned int written = mWriteBuffer.write(data, size);
//    size -= written;
//    data += written;
//    triggerWrite();
//    }   while (size > 0);


