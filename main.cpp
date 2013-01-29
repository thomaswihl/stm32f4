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
#include "Power.h"

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

    gSys.mRcc.enable(ClockControl::Function::GpioA);
    gSys.mRcc.enable(ClockControl::Function::GpioE);
    // SCK
    gSys.mGpioA.configOutput(Gpio::Index::Pin5, Gpio::OutputType::PushPull);
    gSys.mGpioA.setAlternate(Gpio::Index::Pin5, Gpio::AltFunc::SPI1);
    // MISO
    gSys.mGpioA.configInput(Gpio::Index::Pin6);
    gSys.mGpioA.setAlternate(Gpio::Index::Pin6, Gpio::AltFunc::SPI1);
    // MOSI
    gSys.mGpioA.configOutput(Gpio::Index::Pin7, Gpio::OutputType::PushPull);
    gSys.mGpioA.setAlternate(Gpio::Index::Pin7, Gpio::AltFunc::SPI1);
    // CS
    gSys.mGpioE.configOutput(Gpio::Index::Pin3, Gpio::OutputType::PushPull);
    // INT1
    gSys.mGpioE.configInput(Gpio::Index::Pin0);
    gSys.mRcc.enable(ClockControl::Function::SysCfg);
    gSys.mSysCfg.extIntSource(Gpio::Index::Pin0, SysCfg::Gpio::E);
    InterruptController::Line extInt0(gSys.mNvic, StmSystem::InterruptIndex::EXTI0);
    extInt0.setCallback(&gSys.mExtI);
    extInt0.enable();
    // INT2
    gSys.mGpioE.configInput(Gpio::Index::Pin1);
    gSys.mSysCfg.extIntSource(Gpio::Index::Pin1, SysCfg::Gpio::E);
    InterruptController::Line extInt1(gSys.mNvic, StmSystem::InterruptIndex::EXTI1);
    extInt1.setCallback(&gSys.mExtI);
    extInt1.enable();

    gSys.mRcc.enable(ClockControl::Function::Spi1);

    gSys.mSpi1.configChipSelect(new Gpio::Pin(gSys.mGpioE, Gpio::Index::Pin3), true);
    gSys.mSpi1.configDma(new Dma::Stream(gSys.mDma2, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel3,
                                         new InterruptController::Line(gSys.mNvic, StmSystem::InterruptIndex::DMA2_Stream3)),
                         new Dma::Stream(gSys.mDma2, Dma::Stream::StreamIndex::Stream2, Dma::Stream::ChannelIndex::Channel3,
                                         new InterruptController::Line(gSys.mNvic, StmSystem::InterruptIndex::DMA2_Stream2))
                         );

    LIS302DL lis(gSys.mSpi1);
    lis.configInterrupt(new ExternalInterrupt::Line(gSys.mExtI, 0), new ExternalInterrupt::Line(gSys.mExtI, 1));
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
    interpreter.add(new CmdLis(lis));

    InterruptController::Line timer11Irq(gSys.mNvic, StmSystem::InterruptIndex::TIM1_TRG_COM_TIM11);
    Timer timer11(StmSystem::BaseAddress::TIM11, timer11Irq);
    Power pwr(StmSystem::BaseAddress::PWR);
    pwr.backupDomainWp(false);
    gSys.mRcc.enableRtc(ClockControl::RtcClock::HighSpeedExternal);
    pwr.backupDomainWp(true);
    interpreter.add(new CmdMeasureClock(gSys.mRcc, timer11));

    Gpio* gpio[] = { &gSys.mGpioA, &gSys.mGpioB, &gSys.mGpioC, &gSys.mGpioD, &gSys.mGpioE, &gSys.mGpioF, &gSys.mGpioG, &gSys.mGpioH, &gSys.mGpioI };
    interpreter.add(new CmdPin(gpio, sizeof(gpio) / sizeof(gpio[0])));

    interpreter.start();

    gSys.mIWdg.enable(2000000);
    System::Event* event;
    while (true)
    {
        if (gSys.waitForEvent(event) && event != nullptr)
        {
            gSys.mIWdg.service();
            gSys.mGpioD.set(Gpio::Index::Pin13);
            event->callback();
            gSys.mGpioD.reset(Gpio::Index::Pin13);
        }
    }

    return 0;

}



