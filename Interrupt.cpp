#include "Interrupt.h"

#include <cstdio>

Interrupt::Interrupt(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile EXTI*>(base))
{
    mHandler = new Handler*[vectorSize];
}

Interrupt::~Interrupt()
{
}

void Interrupt::set(Index index, Handler *handler)
{
    mHandler[index] = handler;
}


void Interrupt::handle(Interrupt::Index index)
{
    if (mHandler[index] != 0) mHandler[index]->handle(index);
}
