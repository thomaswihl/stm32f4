#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "InterruptController.h"

#include <cstdint>

class ExternalInterrupt
{
public:
    ExternalInterrupt(unsigned int base, std::size_t vectorSize);
    ~ExternalInterrupt();

    void set(InterruptController::Index index, InterruptController::Handler *handler);
    void handle(InterruptController::Index index);
private:
    struct EXTI
    {
        uint32_t IMR : 23;
        uint32_t : 0;
        uint32_t EMR : 23;
        uint32_t : 0;
    };

    volatile EXTI* mBase;
    InterruptController::Handler** mHandler;

};

#endif // INTERRUPT_H
