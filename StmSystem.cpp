#include "StmSystem.h"

#include <cstdio>

extern "C"
{
void __attribute__((interrupt)) Isr()
{
    register int index __asm("r0");
    __asm volatile("MRS r0, IPSR\n" ::: "r0", "cc", "memory");
    std::printf("Interrupt: %i\n", index & 0xff);
    System::instance()->handleInterrupt(index & 0xff);
}

extern void (* const gIsrVectorTable[])(void);
__attribute__ ((section(".isr_vector_table")))
void (* const gIsrVectorTable[])(void) = {
    // 16 trap functions for ARM
    (void (* const)())&__stack_end, (void (* const)())&_start, Isr, Isr, Isr, Isr, 0, 0,
    0, 0, Isr, Isr, 0, Isr, Isr,
    // 82 hardware interrupts specific to the STM32F407
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr, Isr,
    Isr, Isr
};

}

StmSystem::StmSystem() :
    mClock(static_cast<System::BaseAddress>(BaseAddress::RCC), 8000000),
    mInt(static_cast<System::BaseAddress>(BaseAddress::EXTI), 82),
    mDebug(static_cast<System::BaseAddress>(BaseAddress::USART2), mClock),
    mFlash(static_cast<System::BaseAddress>(BaseAddress::FLASH), mClock)
{
    init();
}

StmSystem::~StmSystem()
{
}

void StmSystem::init()
{
    mDebug.setBaudrate(115200);
    mFlash.set(Flash::Feature::InstructionCache, true);
    mFlash.set(Flash::Feature::DataCache, true);
    mClock.setSystemClock(168000000);
}

void StmSystem::handleInterrupt(uint32_t index)
{
    if (!System::tryHandleInterrupt(index)) mInt.handle(index);
}

void StmSystem::debugRead(char *msg, int len)
{
    System::Buffer buf(msg, len);
    mDebug.read(buf);
}

void StmSystem::debugWrite(const char *msg, int len)
{
    System::Buffer buf(msg, len);
    mDebug.write(buf);
}
