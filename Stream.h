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

#ifndef STREAM_H
#define STREAM_H

#include "System.h"
#include "CircularBuffer.h"

class Device
{
public:
    enum class Part { Read, Write, All };

    virtual void enable(Part part) = 0;
    virtual void disable(Part part) = 0;
};

template<typename T>
class Stream : public Device
{
public:
    Stream(System& system);
    virtual void read(T* data, unsigned int count) = 0;
    virtual void read(T* data, unsigned int count, System::Event* callback) = 0;

    virtual void write(const T* data, unsigned int count) = 0;
    virtual void write(const T* data, unsigned int count, System::Event* callback) = 0;

protected:
    System& mSystem;
    T* mReadData;
    unsigned int mReadCount;
    System::Event* mReadCallback;
    const T* mWriteData;
    unsigned int mWriteCount;
    System::Event* mWriteCallback;

    virtual void readPrepare(T* data, unsigned int count);
    virtual void readPrepare(T* data, unsigned int count, System::Event* callback);
    virtual bool read(T data);
    virtual void readFinished(bool success);

    virtual void writePrepare(const T* data, unsigned int count);
    virtual void writePrepare(const T* data, unsigned int count, System::Event* callback);
    virtual bool write(T& data);
    virtual void writeFinished(bool success);

private:
};

template<typename T>
class BufferedStream : public Stream<T>
{
public:
    BufferedStream(Stream<T>& stream, unsigned int readBufferSize, unsigned int writeBufferSize) :
        mStream(stream),
        mReadBuffer(readBufferSize),
        mWriteBuffer(writeBufferSize)
    {

    }

    virtual void read(T* data, unsigned int count);
    virtual void read(T* data, unsigned int count, System::Event* callback);

    virtual void write(const T* data, unsigned int count) = 0;
    virtual void write(const T* data, unsigned int count, System::Event* callback);
protected:
    void finished(bool read, bool success);
private:
    class EventCallback : public System::Event
    {
    public:
        EventCallback(BufferedStream& stream, bool read) : mBufferedStream(stream), mRead(read) { }
        virtual void eventCallback(bool success) { mBufferedStream.finished(mRead, success); }
    private:
        BufferedStream& mBufferedStream;
        bool mRead;
    };

    Stream<T>& mStream;
    CircularBuffer<T> mReadBuffer;
    CircularBuffer<T> mWriteBuffer;
};

#endif // STREAM_H
