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
#include "hw/ws2801.h"
#include "hw/ssd1306.h"
#include "hw/tlc5940.h"
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

    // clock command
    InterruptController::Line timer11Irq(gSys.mNvic, StmSystem::InterruptIndex::TIM1_TRG_COM_TIM11);
    Timer timer11(StmSystem::BaseAddress::TIM11);
    timer11.setInterrupt(Timer::InterruptType::CaptureCompare, &timer11Irq);
    Power pwr(StmSystem::BaseAddress::PWR);
    pwr.backupDomainWp(false);
    gSys.mRcc.enableRtc(ClockControl::RtcClock::HighSpeedExternal);
    pwr.backupDomainWp(true);
    interpreter.add(new CmdMeasureClock(gSys.mRcc, timer11));

    // pin command
    Gpio* gpio[] = { &gSys.mGpioA, &gSys.mGpioB, &gSys.mGpioC, &gSys.mGpioD, &gSys.mGpioE, &gSys.mGpioF, &gSys.mGpioG, &gSys.mGpioH, &gSys.mGpioI };
    interpreter.add(new CmdPin(gpio, sizeof(gpio) / sizeof(gpio[0])));

    // rgb command
    gSys.mRcc.enable(ClockControl::Function::Spi2);
    gSys.mRcc.enable(ClockControl::Function::GpioB);
    // SCK
    gSys.mGpioB.configOutput(Gpio::Index::Pin13, Gpio::OutputType::PushPull);
    gSys.mGpioB.setAlternate(Gpio::Index::Pin13, Gpio::AltFunc::SPI2);
    // MOSI
    gSys.mGpioB.configOutput(Gpio::Index::Pin15, Gpio::OutputType::PushPull);
    gSys.mGpioB.setAlternate(Gpio::Index::Pin15, Gpio::AltFunc::SPI2);

    gSys.mSpi2.configDma(new Dma::Stream(gSys.mDma1, Dma::Stream::StreamIndex::Stream4, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(gSys.mNvic, StmSystem::InterruptIndex::DMA1_Stream4)),
                         new Dma::Stream(gSys.mDma1, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(gSys.mNvic, StmSystem::InterruptIndex::DMA1_Stream3))
                         );

    Ws2801 ws(gSys.mSpi2, 4);
    ws.enable();
    interpreter.add(new CmdRgb(ws));

    Gpio::Pin xlat(gSys.mGpioB, Gpio::Index::Pin11);
    Gpio::Pin blank(gSys.mGpioB, Gpio::Index::Pin12);
    Gpio::Pin gsclk(gSys.mGpioB, Gpio::Index::Pin14);
    gSys.mGpioB.configOutput(Gpio::Index::Pin11, Gpio::OutputType::PushPull);
    gSys.mGpioB.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull);
    gSys.mGpioB.configOutput(Gpio::Index::Pin14, Gpio::OutputType::PushPull);
    //gSys.mGpioB.setAlternate(Gpio::Index::Pin14, Gpio::AltFunc::TIM1);
    Timer gsclkTimer(StmSystem::BaseAddress::TIM1);
    InterruptController::Line timer1IrqUpdate(gSys.mNvic, StmSystem::InterruptIndex::TIM1_UP_TIM10);
    gsclkTimer.setInterrupt(Timer::InterruptType::Update, &timer1IrqUpdate);
    InterruptController::Line timer1IrqCc(gSys.mNvic, StmSystem::InterruptIndex::TIM1_CC);
    gsclkTimer.setInterrupt(Timer::InterruptType::CaptureCompare, &timer1IrqCc);
    Tlc5940 tlc(gSys.mSpi2, xlat, blank, gsclkTimer);
    tlc.setOutput(0, 0xfff);
    tlc.setOutput(1, 0xfff);
    tlc.setOutput(2, 0xfff);
    tlc.setOutput(3, 0xfff);
    tlc.send();
    gsclk.set();
    gSys.nspin(100);
    gsclk.reset();
    gSys.nspin(100);
    gsclk.set();
    gSys.nspin(100);
    gsclk.reset();
    gSys.nspin(100);
//    Gpio::Pin dataCommand(gSys.mGpioB, Gpio::Index::Pin11);
//    Gpio::Pin cs(gSys.mGpioB, Gpio::Index::Pin12);
//    Gpio::Pin reset(gSys.mGpioB, Gpio::Index::Pin14);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin11, Gpio::OutputType::PushPull);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin12, Gpio::OutputType::PushPull);
//    gSys.mGpioB.configOutput(Gpio::Index::Pin14, Gpio::OutputType::PushPull);
//    Ssd1306 oled(gSys.mSpi2, cs, dataCommand, reset);
//    oled.init();

    gSys.mRcc.enable(ClockControl::Function::GpioE);
    gSys.mRcc.enable(ClockControl::Function::GpioA);
    gSys.mRcc.enable(ClockControl::Function::Tim1);
    gSys.mGpioE.configOutput(Gpio::Index::Pin0, Gpio::OutputType::PushPull);
    gSys.mGpioE.configOutput(Gpio::Index::Pin1, Gpio::OutputType::PushPull);
    gSys.mGpioE.configOutput(Gpio::Index::Pin2, Gpio::OutputType::PushPull);
    gSys.mGpioA.configInput(Gpio::Index::Pin8);
    gSys.mGpioA.setAlternate(Gpio::Index::Pin8, Gpio::AltFunc::TIM1);
    Gpio::Pin led(gSys.mGpioE, Gpio::Index::Pin0);
    Gpio::Pin s2(gSys.mGpioE, Gpio::Index::Pin1);
    Gpio::Pin s3(gSys.mGpioE, Gpio::Index::Pin2);
    InterruptController::Line timer1Irq(gSys.mNvic, StmSystem::InterruptIndex::TIM1_CC);
    Timer timer1(StmSystem::BaseAddress::TIM1);
    timer1.setInterrupt(Timer::InterruptType::CaptureCompare, &timer1Irq);
    interpreter.add(new CmdLightSensor(led, s2, s3, timer1, ws));


    interpreter.start();

    //gSys.mIWdg.enable(2000000);
    System::Event* event;
    while (true)
    {
        if (gSys.waitForEvent(event) && event != nullptr)
        {
            //gSys.mIWdg.service();
            gSys.mGpioD.set(Gpio::Index::Pin13);
            event->callback();
            gSys.mGpioD.reset(Gpio::Index::Pin13);
        }
    }

    return 0;

}



