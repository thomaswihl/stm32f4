#ifndef DS18B20_H
#define DS18B20_H

#include "../System.h"
#include "../Gpio.h"

class Ds18b20
{
public:
    Ds18b20(Gpio::Pin& oneWire);

    int temp();

private:
    Gpio::Pin& mOneWire;
    unsigned A;
    unsigned B;
    unsigned C;
    unsigned D;
    unsigned E;
    unsigned F;
    unsigned G;
    unsigned H;
    unsigned I;
    unsigned J;

    void setOverdriveSpeed(bool overdrive);
    void usleep(unsigned us) { System::instance()->usleep(us); }
    bool reset();
    void writeBit(bool bit);
    bool readBit();
    void writeByte(uint8_t byte);
    uint8_t readByte();
};

#endif // DS18B20_H
