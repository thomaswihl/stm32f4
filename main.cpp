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
#include "CommandInterpreter.h"

#include <cstdio>


StmSystem gSys;
CommandInterpreter gCmd;

void handleSerialEvent(System::Event::Component component, Serial::EventType event)
{
    switch (event)
    {
    case Serial::EventType::ReceivedByte:
        char c;
        gSys.mDebug.read(&c, 1);
        gSys.mDebug.write(&c, 1);
        //gCmd.append(c);
        if (c == '\r')
        {
            const char* t = "\n# ";
            gSys.mDebug.write(t, 3);
        }
        else if (c == 18)
        {
            gSys.mRcc.resetClock();
            gSys.printInfo();
        }
        break;
    }
}

int main()
{
    gSys.printInfo();
    gSys.mRcc.enable(ClockControl::Function::GpioD);

    gSys.mGpioD.configOutput(Gpio::Pin::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin13, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin14, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin15, Gpio::OutputType::PushPull);

    gSys.mGpioD.set(Gpio::Pin::Pin12);
    std::printf("# ");
    std::fflush(stdout);
    System::Event event;
    while (true)
    {
        if (gSys.waitForEvent(event))
        {
            gSys.mGpioD.reset(Gpio::Pin::Pin12);
            switch (event.component())
            {
            case System::Event::Component::USART2:
                handleSerialEvent(event.component(), static_cast<Serial::EventType>(event.type()));
                break;
            default:
                break;
            }
            gSys.mGpioD.set(Gpio::Pin::Pin12);
        }
    }

    return 0;

}



