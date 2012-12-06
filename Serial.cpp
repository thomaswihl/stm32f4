#include "Serial.h"

#include <cassert>

Serial::Serial(Base base, System::InterruptIndex irq) : mBase(reinterpret_cast<volatile Usart*>(base))
{
    System* sys = System::instance();
    sys->setInterrupt(irq, this);
}

Serial::~Serial()
{
    mBase->SR.NF = 1;
    mBase->DR = 2;
    mBase->CR1.SBK = 1;

}

void Serial::irq(int index)
{
//    if (USART_GetITStatus(USART2, USART_IT_RXNE))
//    {
//        // read byte and clear interrupt bit
//        char c = USART_ReceiveData(USART2);
//        _write(1, &c, 1);
//        if (c == '\r')
//        {
//            c = '\n';
//            _write(1, &c, 1);
//        }
//    }
}

