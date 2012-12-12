#include "InterruptController.h"

InterruptController::InterruptController(unsigned int base, std::size_t vectorSize) :
    mBase(reinterpret_cast<volatile NVIC*>(base))
{
    static_assert(sizeof(NVIC) == 0xe04, "Struct has wrong size, compiler problem.");
    mHandler = new Handler*[vectorSize];
}

InterruptController::~InterruptController()
{
}

void InterruptController::handle(Index index)
{
    if (mHandler[index] != 0) mHandler[index]->handle(index);
}

void InterruptController::setPriotity(InterruptController::Index index, InterruptController::Priority priority)
{
    mBase->IPR[index] = static_cast<uint8_t>(priority) << 4;
}

InterruptController::Line::Line(InterruptController &interruptController, InterruptController::Index index) :
    mInterruptController(interruptController),
    mIndex(index)
{
}

InterruptController::Line::~Line()
{
    disable();
    setHandler(0);
}

void InterruptController::Line::setHandler(InterruptController::Handler *handler)
{
    mInterruptController.mHandler[mIndex] = handler;
}

void InterruptController::Line::enable()
{
    mInterruptController.mBase->ISER[mIndex / 32] = 1 << (mIndex % 32);
}

void InterruptController::Line::disable()
{
    mInterruptController.mBase->ICER[mIndex / 32] = 1 << (mIndex % 32);
}
