

#include "StmSystem.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"

#include <cstdio>


//extern "C"
//{
//    void __attribute__((interrupt)) USART2_IRQHandler(void)
//    {
//        if (USART_GetITStatus(USART2, USART_IT_RXNE))
//        {
//            // read byte and clear interrupt bit
//            char c = USART_ReceiveData(USART2);
//            _write(1, &c, 1);
//            if (c == '\r')
//            {
//                c = '\n';
//                _write(1, &c, 1);
//            }
//        }
//    }
//}


StmSystem system;

int main()
{
    std::printf("Welcome!\n");

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    /* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15);

    while (true)
    {

    }

    return 0;

}



