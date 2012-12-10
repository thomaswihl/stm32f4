#include "StmSystem.h"

#include <cstdio>

StmSystem system;

void sleep()
{
    for (int i = 0; i < 1000000; ++i)
    {

    }
}

int main()
{
//    std::printf("System clock is %.3fMHz, AHB clock is %.3fMHz, APB1 is %.3fMHz, APB2 is %.3fMHz\n",
//                system.mRcc.clock(ClockControl::Clock::System) / 1000000.0f,
//                system.mRcc.clock(ClockControl::Clock::AHB) / 1000000.0f,
//                system.mRcc.clock(ClockControl::Clock::APB1) / 1000000.0f,
//                system.mRcc.clock(ClockControl::Clock::APB2) / 1000000.0f);
    std::printf("System clock is %luMHz, AHB clock is %luMHz, APB1 is %luMHz, APB2 is %luMHz.\n",
                system.mRcc.clock(ClockControl::Clock::System) / 1000000,
                system.mRcc.clock(ClockControl::Clock::AHB) / 1000000,
                system.mRcc.clock(ClockControl::Clock::APB1) / 1000000,
                system.mRcc.clock(ClockControl::Clock::APB2) / 1000000);
    std::printf("RAM  : %luk free, %luk used.\n", system.memFree() / 1024, system.memUsed() / 1024);
    std::printf("STACK: %luk free, %luk used.\n", system.stackFree() / 1024, system.stackUsed() / 1024);

    system.mRcc.enable(ClockControl::Function::GpioD);

    system.mGpioD.configOutput(Gpio::Pin::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    system.mGpioD.configOutput(Gpio::Pin::Pin13, Gpio::OutputType::PushPull);
    system.mGpioD.configOutput(Gpio::Pin::Pin14, Gpio::OutputType::PushPull);
    system.mGpioD.configOutput(Gpio::Pin::Pin15, Gpio::OutputType::PushPull);

    while (true)
    {
        system.mGpioD.set(Gpio::Pin::Pin12);
        sleep();
        system.mGpioD.set(Gpio::Pin::Pin13);
        sleep();
        system.mGpioD.set(Gpio::Pin::Pin14);
        sleep();
        system.mGpioD.set(Gpio::Pin::Pin15);
        sleep();
        system.mGpioD.reset(Gpio::Pin::Pin12);
        sleep();
        system.mGpioD.reset(Gpio::Pin::Pin13);
        sleep();
        system.mGpioD.reset(Gpio::Pin::Pin14);
        sleep();
        system.mGpioD.reset(Gpio::Pin::Pin15);
        sleep();
    }

    return 0;

}



