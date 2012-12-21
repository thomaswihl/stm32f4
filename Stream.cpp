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
void Stream<T>::readPrepare(T *data, unsigned int count)
{
    while (mReadCount != 0) ;
    mReadData = data;
    mReadCount = count;
}

template<typename T>
void Stream<T>::readPrepare(T *data, unsigned int count, System::Event *callback)
{
    readPrepare(data, count);
    mReadCallback = callback;
}

template<typename T>
bool Stream<T>::read(T data)
{
    if (mReadCount != 0)
    {
        *mReadData++ = data;
        --mReadCount;
    }
    return mReadCount == 0;
}

template<typename T>
void Stream<T>::readFinished()
{
    if (mReadCallback != nullptr)
    {
        mSystem.postEvent(mReadCallback);
        mReadCallback = nullptr;
    }
}

template<typename T>
void Stream<T>::writePrepare(const T *data, unsigned int count)
{
    while (mWriteCount != 0) ;
    mWriteData = data;
    mWriteCount = count;
}

template<typename T>
void Stream<T>::writePrepare(const T *data, unsigned int count, System::Event *callback)
{
    writePrepare(data, count);
    mWriteCallback = callback;
}

template<typename T>
bool Stream<T>::write(T &data)
{
    if (mWriteCount != 0)
    {
        data = *mWriteData++;
        --mWriteCount;
    }
    return mWriteCount == 0;
}

template<typename T>
void Stream<T>::writeFinished()
{
    if (mWriteCallback != nullptr)
    {
        mSystem.postEvent(mWriteCallback);
        mWriteCallback = nullptr;
    }
}

template class Stream<char>;
template class Stream<uint16_t>;


template<typename T>
void BufferedStream<T>::read(T *data, unsigned int count, System::Event *callback)
{
}

template<typename T>
void BufferedStream<T>::read(T *data, unsigned int count)
{
    while (mReadBuffer.used() == 0)
    {
        __asm("wfi");
    }

    mReadBuffer.read(data, count);
}


//do
//{
//    unsigned int written = mWriteBuffer.write(data, size);
//    size -= written;
//    data += written;
//    triggerWrite();
//    }   while (size > 0);


