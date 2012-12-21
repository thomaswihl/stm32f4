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

template<typename T>
class CircularBuffer
{
public:
    CircularBuffer(unsigned int size);
    ~CircularBuffer();

    inline unsigned int used() { return mUsed; }
    inline unsigned int free() { return mSize - mUsed; }
    inline unsigned int size() { return mSize; }

    bool push(T c);
    bool pop(T &c);
    unsigned int write(const T* data, unsigned int len);
    unsigned int read(T* data, unsigned int len);


    unsigned int getContBuffer(const T*& data);

    unsigned int skip(unsigned int len);

protected:
    unsigned int mSize;
    T* mBuffer;
    T* mWrite;
    T* mRead;
    volatile unsigned int mUsed;

    unsigned int writePart(const T* data, unsigned int len);
    unsigned int readPart(T *data, unsigned int len);

    inline void align(T*volatile& ptr) { if (ptr >= (mBuffer + mSize)) ptr = mBuffer; }

    friend int testCircularBuffer();

};

#endif // CIRCULARBUFFER_H
