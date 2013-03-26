#include "ds18b20.h"

Ds18b20::Ds18b20(Gpio::Pin &oneWire) :
    mOneWire(oneWire)
{
    setOverdriveSpeed(false);
}

int Ds18b20::temp()
{
    if (reset())
    {
        writeByte(0xcc); // skip ROM, only one device
        writeByte(0x44); // convert

        while (!mOneWire.get())
        {

        }

        reset();
        writeByte(0xcc); // skip ROM, only one device
        writeByte(0xbe);

        uint8_t buf[9];
        for (int i = 0; i < 9; ++i) buf[i] = readByte();

        printf("SP:");
        for (int i = 0; i < 9; ++i) printf(" %02x", buf[i]);
        printf("\n");
        return (buf[1] << 8) | buf[0];
    }
    else
    {
        printf("No device found.\n");
    }
    return 0;
}

void Ds18b20::setOverdriveSpeed(bool overdrive)
{
    if (!overdrive)
    {
        // Standard Speed
        A = 6;
        B = 64;
        C = 60;
        D = 10;
        E = 9;
        F = 55;
        G = 0;
        H = 480;
        I = 70;
        J = 410;
    }
    else
    {
        // Overdrive Speed
        A = 1.5;
        B = 7.5;
        C = 7.5;
        D = 2.5;
        E = 0.75;
        F = 7;
        G = 2.5;
        H = 70;
        I = 8.5;
        J = 40;
    }
}

bool Ds18b20::reset()
{
    usleep(G);
    mOneWire.reset();
    usleep(H);
    mOneWire.set();
    usleep(I);
    bool present = !mOneWire.get();
    usleep(J);
    return present;
}

void Ds18b20::writeBit(bool bit)
{
    mOneWire.reset();
    usleep(bit ? A : C);
    mOneWire.set();
    usleep(bit ? B : D);
}

bool Ds18b20::readBit()
{
    mOneWire.reset();
    usleep(A);
    mOneWire.set();
    usleep(E);
    bool bit = mOneWire.get();
    usleep(F);
    return bit;
}

void Ds18b20::writeByte(uint8_t byte)
{
    for (int i = 0; i < 8; ++i) writeBit((byte >> i) & 1);
}

uint8_t Ds18b20::readByte()
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; ++i) byte |= readBit() ? (1 << i) : 0;
    return byte;
}
