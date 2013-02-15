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
    System::instance()->usleep(15000);
    write(0x3); // Switch to 8bit so we can find or start
    write(0x3); // Switch to 8bit so we can find or start
    write(0x3); // Switch to 8bit so we can find or start
    write(0x28); // DL = 0 -> 4bit, N = 1 -> 2 line, F = 0 -> 5x8
    write(0x06); // Increase, no shift
    write(0x0c); // Display on, cursor off
}

void Adm1602::write(const char* str, unsigned len)
{
    if (len > 0x20) len = 0x20;
    while (len > 0)
    {
        write(*str++, true);
        --len;
    }
}

void Adm1602::moveTo(int addr)
{
    write(addr | 0x80);
}

void Adm1602::clear()
{
    write(0x01);
    System::instance()->usleep(1600);
}

void Adm1602::home()
{
    write(0x02);
    System::instance()->usleep(1600);
}

void Adm1602::cursor(bool on, bool blink)
{
    write(0x0c | (on ? 2 : 0) | (blink ? 1 : 0));
}

void Adm1602::write(uint8_t data, bool rs)
{
    if (mRs.get() != rs)
    {
        mRs.set(rs);
        System::instance()->nspin(100);
    }
    mEnable.set();
    for (int i = 0; i < 4; ++i) mData[i]->set((data & (16 << i)) != 0);
    System::instance()->nspin(300); // Eneable set -> reset > 300ns, Data valid -> Enable reset > 60ns
    mEnable.reset();
    System::instance()->nspin(200); // Enable cycle > 500ns
    mEnable.set();
    for (int i = 0; i < 4; ++i) mData[i]->set((data & (1 << i)) != 0);
    System::instance()->nspin(300); // Eneable set -> reset > 300ns, Data valid -> Enable reset > 60ns
    mEnable.reset();
    System::instance()->usleep(40);
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
