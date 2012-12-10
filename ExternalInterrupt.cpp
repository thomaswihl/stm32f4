#include "ExternalInterrupt.h"

#include <cstdio>

ExternalInterrupt::ExternalInterrupt(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile EXTI*>(base))
{
    mHandler = new InterruptController::Handler*[vectorSize];
}

ExternalInterrupt::~ExternalInterrupt()
{
}

void ExternalInterrupt::set(InterruptController::Index index, InterruptController::Handler *handler)
{
    mHandler[index] = handler;
}


void ExternalInterrupt::handle(InterruptController::Index index)
{
    if (mHandler[index] != 0) mHandler[index]->handle(index);
}
