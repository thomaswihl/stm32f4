#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

class CircularBuffer
{
public:
    CircularBuffer(unsigned int size);
    ~CircularBuffer();

    unsigned int size();
    unsigned int free();

    bool push(char c);
    char pop();

    unsigned int append(const char* data, unsigned int len);
    unsigned int getContBuffer(char*& data);
protected:
    unsigned int mSize;
    char* mBuffer;
    char* mNextWrite;
    char* mLastRead;

    unsigned int appendPart(const char* data, unsigned int len);
};

#endif // CIRCULARBUFFER_H
