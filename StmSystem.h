#ifndef STMSYSTEM_H
#define STMSYSTEM_H

#include "System.h"
#include "ClockControl.h"
#include "Interrupt.h"
#include "Serial.h"

class StmSystem : public System
{
public:
    enum class BaseAddress : unsigned long
    {
        RCC = 0x40023800,
        EXTI = 0x40013c00,
        USART1 = 0x40011000,
        USART2 = 0x40004400,
        USART3 = 0x40004800,
        UART4 = 0x40004c00,
        UART5 = 0x40005000,
        USART6 = 0x40011400,
    };

    enum class InterruptIndex : Interrupt::Index
    {
        WWDG,
        PVD,
        TAMP_STAMP,
        RTC_WKUP,
        FLASH,
        RCC,
        EXTI0,
        EXTI1,
        EXTI2,
        EXTI3,
        EXTI4,
        DMA1_Stream0,
        DMA1_Stream1,
        DMA1_Stream2,
        DMA1_Stream3,
        DMA1_Stream4,
        DMA1_Stream5,
        DMA1_Stream6,
        ADC,
        CAN1_TX,
        CAN1_RX0,
        CAN1_RX1,
        CAN1_SCE,
        EXTI9_5,
        TIM1_BRK_TIM9,
        TIM1_UP_TIM10,
        TIM1_TRG_COM_TIM11,
        TIM1_CC,
        TIM2,
        TIM3,
        TIM4,
        I2C1_EV,
        I2C1_ER,
        I2C2_EV,
        I2C2_ER,
        SPI1,
        SPI2,
        USART1,
        USART2,
        USART3,
        EXTI15_10,
        RTC_Alarm,
        OTG_FS_WKUP,
        TIM8_BRK_TIM12,
        TIM8_UP_TIM13,
        TIM8_TRG_COM_TIM14,
        TIM8_CC,
        DMA1_Stream7,
        FSMC,
        SDIO,
        TIM5,
        SPI3,
        UART4,
        UART5,
        TIM6_DAC,
        TIM7,
        DMA2_Stream0,
        DMA2_Stream1,
        DMA2_Stream2,
        DMA2_Stream3,
        DMA2_Stream4,
        ETH,
        ETH_WKUP,
        CAN2_TX,
        CAN2_RX0,
        CAN2_RX1,
        CAN2_SCE,
        OTG_FS,
        DMA2_Stream5,
        DMA2_Stream6,
        DMA2_Stream7,
        USART6,
        I2C3_EV,
        I2C3_ER,
        OTG_HS_EP1_OU,
        OTG_HS_EP1_IN,
        OTG_HS_WKUP,
        OTG_HS,
        DCMI,
        CRYP,
        HASH_RNG,
        FPU,
        __COUNT
    };

    StmSystem();
    virtual ~StmSystem();

    virtual void handleInterrupt(uint32_t index);
private:
    ClockControl mClock;
    Interrupt mInt;
    Serial mDebug;

    void init();
};

#endif // STMSYSTEM_H
