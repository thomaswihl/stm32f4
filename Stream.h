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
#include "Dma.h"

template<typename T>
class Stream
{
public:
    Stream(System& system);
    bool read(T* data, unsigned int count);
    bool read(T* data, unsigned int count, System::Event* completeEvent);

    bool write(const T* data, unsigned int count);
    bool write(const T* data, unsigned int count, System::Event* completeEvent);

    virtual void readFifo(unsigned int size);
    virtual void writeFifo(unsigned int size);
protected:
    System& mSystem;

    virtual void readPrepare() = 0;
    virtual void readSync() = 0;
    virtual void readTrigger() = 0;
    virtual void readDone() = 0;

    void readSuccess(bool success);
    bool read(T data);
    T* readData();
    unsigned int readCount();

    virtual void writePrepare() = 0;
    virtual void writeSync() = 0;
    virtual void writeTrigger() = 0;
    virtual void writeDone() = 0;

    void writeSuccess(bool success);
    bool write(T& data);
    const T *writeData();
    unsigned int writeCount();

private:
    T* mReadData;
    unsigned int mReadCount;
    System::Event* mReadCompleteEvent;
    const T* mWriteData;
    int mWriteCount;
    System::Event* mWriteCompleteEvent;
    CircularBuffer<T>* mReadFifo;
    CircularBuffer<T>* mWriteFifo;

    bool readProlog(T* data, unsigned int count);
    void readEpilog();
    void readFromFifo(T*& data, unsigned int& count);

    bool writeProlog(const T* data, unsigned int count);
    void writeEpilog();
    void writeToFifo(const T *&data, unsigned int &count);
};

#endif // STREAM_H
