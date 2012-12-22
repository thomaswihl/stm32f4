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
#include "Commands.h"
#include "hw/lis302dl.h"

#include <cstdio>
#include <memory>


StmSystem gSys;

int main()
{
    LIS302DL lis(gSys.mSpi1);
    CommandInterpreter interpreter(gSys);

    printf("\n\n\nRESET\n");
    gSys.printInfo();
    gSys.mRcc.enable(ClockControl::Function::GpioD);

    gSys.mGpioD.configOutput(Gpio::Pin::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin13, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin14, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Pin::Pin15, Gpio::OutputType::PushPull);

    gSys.mGpioD.set(Gpio::Pin::Pin12);
    interpreter.add(new CmdHelp());
    interpreter.add(new CmdInfo(gSys));
    interpreter.add(new CmdFunc(gSys));
    interpreter.add(new CmdRead());
    interpreter.add(new CmdWrite());
    interpreter.add(new CmdLis(lis));
    interpreter.start();

    System::Event* event;
    while (true)
    {
        if (gSys.waitForEvent(event) && event != nullptr)
        {
            gSys.mGpioD.reset(Gpio::Pin::Pin12);
            event->eventCallback(event->success());
            gSys.mGpioD.set(Gpio::Pin::Pin12);
        }
    }

    return 0;

}



