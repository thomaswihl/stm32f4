#ifndef WS2801_H
#define WS2801_H

#include "../Spi.h"
#include "../System.h"

class Ws2801
{
public:
    Ws2801(Spi& spi, unsigned count);
    void enable();
    void set(unsigned index, uint8_t red, uint8_t green, uint8_t blue);

private:
    Spi& mSpi;
    uint8_t* mData;
    unsigned mCount;
    Spi::Transfer mTransfer;
};

#endif // WS2801_H
