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

#ifndef STMSYSTEM_H
#define STMSYSTEM_H

#include "System.h"
#include "ClockControl.h"
#include "Gpio.h"
#include "InterruptController.h"
#include "ExternalInterrupt.h"
#include "Dma.h"
#include "Serial.h"
#include "Flash.h"
#include "SysTickControl.h"
#include "FpuControl.h"
#include "Spi.h"

class StmSystem : public System
{
public:
    struct BaseAddress
    {
        enum Address : System::BaseAddress
        {
            DMA1 = 0x40026000,
            DMA2 = 0x40026400,
            EXTI = 0x40013c00,
            FLASH = 0x40023c00,
            GPIOA = 0x40020000,
            GPIOB = 0x40020400,
            GPIOC = 0x40020800,
            GPIOD = 0x40020c00,
            GPIOE = 0x40021000,
            GPIOF = 0x40021400,
            GPIOG = 0x40021800,
            GPIOH = 0x40021c00,
            GPIOI = 0x40022000,
            NVIC = 0xe000e100,
            RCC = 0x40023800,
            USART1 = 0x40011000,
            USART2 = 0x40004400,
            USART3 = 0x40004800,
            UART4 = 0x40004c00,
            UART5 = 0x40005000,
            USART6 = 0x40011400,
            SPI1 = 0x40013000,
            SPI2 = 0x40003800,
            SPI3 = 0x40003c00,
            STK = 0xe000e010,
            SYSCFG = 0x40013800,
            FPU = 0xe000ed88,
            SCB = 0xe000ed00,
        };
    };

    struct InterruptIndex
    {
        enum Index : InterruptController::Index
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
        };
    };

    Gpio mGpioA;
    Gpio mGpioB;
    Gpio mGpioC;
    Gpio mGpioD;
    Gpio mGpioE;
    Gpio mGpioF;
    Gpio mGpioG;
    Gpio mGpioH;
    Gpio mGpioI;
    ClockControl mRcc;
    ExternalInterrupt mExtI;
    InterruptController mNvic;
    SysTickControl mSysTick;
    Dma mDma1;
    Dma mDma2;
//    Serial mUsart1;
    Serial mUsart2;
//    Serial mUsart3;
//    Serial mUart4;
//    Serial mUart5;
//    Serial mUsart6;
    Serial& mDebug;
    Spi<char> mSpi1;
//    Spi mSpi2;
//    Spi mSpi3;
    Flash mFlash;
    FpuControl mFpu;


    StmSystem();
    virtual ~StmSystem();

    virtual inline void handleInterrupt(uint32_t index) { mNvic.handle(index); }
    virtual void handleTrap(System::TrapIndex index, unsigned int* stackPointer);

    void printInfo();
    virtual void usleep(unsigned int us) { mSysTick.usleep(us); }
    virtual uint64_t ns() { return mSysTick.ns(); }
protected:
    virtual void debugRead(char *msg, unsigned int len);
    virtual void debugWrite(const char *msg, unsigned int len);

private:
    void init();
};

#endif // STMSYSTEM_H
