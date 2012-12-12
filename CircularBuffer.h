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
