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

class CircularBuffer
{
public:
    CircularBuffer(unsigned int size);
    ~CircularBuffer();

    inline unsigned int used() { return mUsed; }
    inline unsigned int free() { return mSize - mUsed; }

    bool push(char c);
    bool pop(char &c);

    unsigned int write(const char* data, unsigned int len);
    unsigned int read(char *data, unsigned int len);

    unsigned int getContBuffer(char*& data);
    unsigned int skip(unsigned int len);
protected:
    unsigned int mSize;
    char* mBuffer;
    char* mWrite;
    char* mRead;
    unsigned int mUsed;

    unsigned int writePart(const char* data, unsigned int len);
    unsigned int readPart(char *data, unsigned int len);
    inline void align(char*& ptr) { if (ptr >= (mBuffer + mSize)) ptr = mBuffer; }

    friend int testCircularBuffer();

};

#endif // CIRCULARBUFFER_H
