/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StmSystem.h"

#include <cstdio>


StmSystem system;

void handleSerialEvent(System::Event::Component component, Serial::EventType event)
{
    switch (event)
    {
    case Serial::EventType::ReceivedByte:
        char c;
        system.mDebug.read(&c, 1);
        system.mDebug.write(&c, 1);
        if (c == '\r')
        {
            c = '\n';
            system.mDebug.write(&c, 1);
        }
        break;
    }
}

int main()
{
    std::printf("System clock is %.3fMHz, AHB clock is %.3fMHz, APB1 is %.3fMHz, APB2 is %.3fMHz\n",
                system.mRcc.clock(ClockControl::Clock::System) / 1000000.0f,
                system.mRcc.clock(ClockControl::Clock::AHB) / 1000000.0f,
                system.mRcc.clock(ClockControl::Clock::APB1) / 1000000.0f,
                system.mRcc.clock(ClockControl::Clock::APB2) / 1000000.0f);
//    std::printf("\nSystem clock is %luMHz, AHB clock is %luMHz, APB1 is %luMHz, APB2 is %luMHz.\n",
//                system.mRcc.clock(ClockControl::Clock::System) / 1000000,
//                system.mRcc.clock(ClockControl::Clock::AHB) / 1000000,
//                system.mRcc.clock(ClockControl::Clock::APB1) / 1000000,
//                system.mRcc.clock(ClockControl::Clock::APB2) / 1000000);
    std::printf("RAM  : %luk free, %luk used.\n", system.memFree() / 1024, system.memUsed() / 1024);
    std::printf("STACK: %luk free, %luk used.\n", system.stackFree() / 1024, system.stackUsed() / 1024);

    system.mRcc.enable(ClockControl::Function::GpioD);

    system.mGpioD.configOutput(Gpio::Pin::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    system.mGpioD.configOutput(Gpio::Pin::Pin13, Gpio::OutputType::PushPull);
    system.mGpioD.configOutput(Gpio::Pin::Pin14, Gpio::OutputType::PushPull);
    system.mGpioD.configOutput(Gpio::Pin::Pin15, Gpio::OutputType::PushPull);

    system.mGpioD.set(Gpio::Pin::Pin12);
    std::printf("# ");
    std::fflush(stdout);
    System::Event event;
    while (true)
    {
        if (system.waitForEvent(event))
        {
            switch (event.component())
            {
            case System::Event::Component::USART2:
                handleSerialEvent(event.component(), static_cast<Serial::EventType>(event.type()));
                break;
            default:
                break;
            }
        }
    }

    return 0;

}



