#include "InterruptController.h"

InterruptController::InterruptController(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile NVIC*>(base))
{
    mHandler = new Handler*[vectorSize];
}

InterruptController::~InterruptController()
{
}

void InterruptController::set(Index index, Handler *handler, Priority priority, bool enabled)
{
    mHandler[index] = handler;
    setPriotity(index, priority);
    if (handler == 0) disable(index);
    else if (enabled) enable(index);
}


void InterruptController::handle(Index index)
{
    if (mHandler[index] != 0) mHandler[index]->handle(index);
}

void InterruptController::enable(InterruptController::Index index)
{
    mBase->ISER[index / 32] = 1 << (index % 32);
}

void InterruptController::disable(InterruptController::Index index)
{
    mBase->ICER[index / 32] = 1 << (index % 32);
}

void InterruptController::setPriotity(InterruptController::Index index, InterruptController::Priority priority)
{
    mBase->IPR[index] = static_cast<uint8_t>(priority) << 4;
}

