#ifndef ADM1602_H
#define ADM1602_H

#include "../Gpio.h"

#include <stdint.h>

class Adm1602
{
public:
    Adm1602(Gpio::Pin& enable, Gpio::Pin& rs, Gpio::Pin& d4, Gpio::Pin& d5, Gpio::Pin& d6, Gpio::Pin& d7);
    void init();
    void write(const char *str, unsigned len);
    void moveTo(int addr);
    void clear();
    void home();
    void cursor(bool on, bool blink);
private:
    Gpio::Pin& mEnable;
    Gpio::Pin& mRs;
    Gpio::Pin& mData4;
    Gpio::Pin& mData5;
    Gpio::Pin& mData6;
    Gpio::Pin& mData7;
    Gpio::Pin* mData[4];

    void write(uint8_t data, bool rs = false);
    void debug();
};

#endif // ADM1602_H
