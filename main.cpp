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
#include "sw/carcontroller.h"
#include "sw/lego.h"

#include <cstdio>
#include <memory>

StmSystem gSys;

int main()
{
    ClockControl::Reset::Reason rr = gSys.mRcc.resetReason();
    printf("\n\n\nRESET: ");
    if (rr & ClockControl::Reset::LowPower) printf("LOW POWER   ");
    if (rr & ClockControl::Reset::WindowWatchdog) printf("WINDOW WATCHDOG   ");
    if (rr & ClockControl::Reset::IndependentWatchdog) printf("INDEPENDENT WATCHDOG   ");
    if (rr & ClockControl::Reset::Software) printf("SOFTWARE RESET   ");
    if (rr & ClockControl::Reset::PowerOn) printf("POWER ON   ");
    if (rr & ClockControl::Reset::Pin) printf("PIN RESET   ");
    if (rr & ClockControl::Reset::BrownOut) printf("BROWN OUT   ");
    printf("\n");
    gSys.printInfo();

    gSys.mRcc.enable(ClockControl::Function::Dma1);
    gSys.mRcc.enable(ClockControl::Function::Dma2);

    CommandInterpreter interpreter(gSys);

    gSys.mRcc.enable(ClockControl::Function::GpioD);

    gSys.mGpioD.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull, Gpio::Pull::None, Gpio::Speed::Fast);
    gSys.mGpioD.configOutput(Gpio::Index::Pin13, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Index::Pin14, Gpio::OutputType::PushPull);
    gSys.mGpioD.configOutput(Gpio::Index::Pin15, Gpio::OutputType::PushPull);

    gSys.mGpioD.set(Gpio::Index::Pin12);
    interpreter.add(new CmdHelp());
    interpreter.add(new CmdInfo(gSys));
    interpreter.add(new CmdFunc(gSys));
    interpreter.add(new CmdRead());
    interpreter.add(new CmdWrite());

    Lego lego;
    lego.init(gSys, interpreter);

    interpreter.start();


    //gSys.mIWdg.enable(2000000);
    System::Event* event;
//    uint64_t old = 0;
    while (true)
    {
        if (gSys.waitForEvent(event) && event != nullptr)
        {
//            if (old == 0 && gSys.mGpioA.get(Gpio::Index::Pin0))
//            {
//                old = System::instance()->ns();
//                if (cc.running()) cc.stop();
//                else cc.start();
//            }
//            else if (old != 0 && !gSys.mGpioA.get(Gpio::Index::Pin0) && System::instance()->ns() - old > 100000000)
//            {
//                old = 0;
//            }
            //gSys.mIWdg.service();
//            gSys.mDebug.write(gSys.mGpioB.get(Gpio::Index::Pin11) ? "1" : "0", 1);

            gSys.mGpioD.set(Gpio::Index::Pin13);
            //printf("Event %p.\n", event);
            event->callback();
            gSys.mGpioD.reset(Gpio::Index::Pin13);
        }
    }

    return 0;

}



