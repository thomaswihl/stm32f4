#include "adm1602.h"

#include "../System.h"

Adm1602::Adm1602(Gpio::Pin &enable, Gpio::Pin& rs, Gpio::Pin &d4, Gpio::Pin &d5, Gpio::Pin &d6, Gpio::Pin &d7) :
    mEnable(enable),
    mRs(rs),
    mData4(d4),
    mData5(d5),
    mData6(d6),
    mData7(d7)
{
    mData[0] = &mData4;
    mData[1] = &mData5;
    mData[2] = &mData6;
    mData[3] = &mData7;
}

void Adm1602::init()
{
    write(0x28); // DL = 0 -> 4bit, N = 1 -> 2 line, F = 0 -> 5x8
    System::instance()->usleep(40);
    write(0x06); // Increase, no shift
    System::instance()->usleep(40);
    write(0x0c); // Display on, cursor off
    System::instance()->usleep(40);
    write(0x01); // clear display
    System::instance()->usleep(40);
}

void Adm1602::write(int addr, const char* str, unsigned len)
{
    write(addr | 0x80);
    if (len > 0x20) len = 0x20;
    while (len > 0)
    {
        write(*str++, true);
        --len;
    }
}

void Adm1602::write(uint8_t data, bool rs)
{
    mRs.set(rs);
    mEnable.set();
    for (int i = 0; i < 4; ++i) mData[i]->set((data & (16 << i)) != 0);
    debug();
    System::instance()->usleep(40);
    mEnable.reset();
    debug();
    System::instance()->usleep(40);
    mEnable.set();
    for (int i = 0; i < 4; ++i) mData[i]->set((data & (1 << i)) != 0);
    debug();
    System::instance()->usleep(40);
    mEnable.reset();
    debug();
    System::instance()->usleep(40);
    mEnable.set();
    mRs.reset();
}

void Adm1602::debug()
{
    return;
    char buf[3];
    char table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    uint8_t data = *((uint8_t*)0x40020410);
    buf[0] = table[data >> 4];
    buf[1] = table[data & 0x0f];
    buf[2] = ' ';
    System::instance()->debugWrite(buf, 3);
}
