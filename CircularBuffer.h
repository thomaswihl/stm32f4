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
    unsigned int read(const char* data, unsigned int len);

    unsigned int getContBuffer(char*& data);
    unsigned int skip(unsigned int len);
protected:
    unsigned int mSize;
    char* mBuffer;
    char* mWrite;
    char* mRead;
    unsigned int mUsed;

    unsigned int appendPart(const char* data, unsigned int len);
    friend void testCircularBuffer();
    inline void align(char*& ptr) { if (ptr >= (mBuffer + mSize)) ptr = mBuffer; }
};

#endif // CIRCULARBUFFER_H
