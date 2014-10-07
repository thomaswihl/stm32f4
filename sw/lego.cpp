#include "../Commands.h"
#include "lego.h"


Lego::Lego()
{
}

void Lego::init(StmSystem &sys, CommandInterpreter &interpreter)
{
    sys.mRcc.enable(ClockControl::Function::Spi2);
    sys.mRcc.enable(ClockControl::Function::GpioB);
    // SS
    sys.mGpioB.configOutput(Gpio::Index::Pin11, Gpio::OutputType::PushPull);
    // SCK
    sys.mGpioB.configOutput(Gpio::Index::Pin13, Gpio::OutputType::PushPull);
    sys.mGpioB.setAlternate(Gpio::Index::Pin13, Gpio::AltFunc::SPI2);
    // MISO
    sys.mGpioB.setAlternate(Gpio::Index::Pin14, Gpio::AltFunc::SPI2);
    // MOSI
    sys.mGpioB.configOutput(Gpio::Index::Pin15, Gpio::OutputType::PushPull);
    sys.mGpioB.setAlternate(Gpio::Index::Pin15, Gpio::AltFunc::SPI2);

    sys.mSpi2.configDma(new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream4, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream4)),
                         new Dma::Stream(sys.mDma1, Dma::Stream::StreamIndex::Stream3, Dma::Stream::ChannelIndex::Channel0,
                                         new InterruptController::Line(sys.mNvic, StmSystem::InterruptIndex::DMA1_Stream3))
                         );
    sys.mSpi2.setMasterSlave(Spi::MasterSlave::Master);
    sys.mSpi2.enable(Device::Part::All);
    mSs = new Gpio::Pin(sys.mGpioB, Gpio::Index::Pin11);

    interpreter.add(new CmdSpiTest(sys.mSpi2, *mSs));
}
