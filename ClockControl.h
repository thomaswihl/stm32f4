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

#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "System.h"

#include <vector>

class ClockControl
{
public:
    class Callback
    {
    public:
        enum Reason { AboutToChange, Changed };
        virtual void clockCallback(Reason reason, uint32_t clock) = 0;
    };
    enum class Function
    {
        GpioA = 0, GpioB, GpioC, GpioD, GpioE, GpioF, GpioG, GpioH, GpioI, Crc = 12, BkpSRam = 18, CcmDataRam = 20, Dma1 = 21, Dma2 = 22, EthMax = 25, EthMaxTx, EthMacRx, EthMacPtp, OtgHs, OtgHsulpi,
        Dcmi = 32, Cryp = 36, Hash, Rng, OtgFs,
        Fsmc = 64,
        Tim2 = 128, Tim3, Tim4, Tim5, Tim6, Tim7, Tim12, Tim13, Tim14, WWdg = 139, Spi2 = 142, Spi3, Usart2 = 145, Usart3, Usart4, Uart5, I2c1, I2c2, I2c3, Can1 = 153, Can2, Pwr = 156, Dac,
        Tim1 = 160, Tim8, Usart1 = 164, Usart6, Adc1 = 168, Adc2, Adc3, Sdio, Spi1, SysCfg = 174, Tim9 = 176, Tim10, Tim11
    };
    struct Reset
    {
        enum Reason { LowPower = 0x80000000, WindowWatchdog = 0x40000000, IndependentWatchdog = 0x20000000, Software = 0x10000000, PowerOn = 0x08000000, Pin = 0x04000000, BrownOut = 0x02000000 };
    };
    enum class Clock { System, AHB, APB1, APB2, RTC };
    enum class AhbPrescaler { by1 = 0, by2 = 8, by4 = 9, by8 = 10, by16 = 11, by64 = 12, by128 = 13, by256 = 14, by512 = 15 };
    enum class Apb1Prescaler { by1 = 0, by2 = 4, by4 = 5, by8 = 6, by16 = 7 };
    enum class Apb2Prescaler { by1 = 0, by2 = 4, by4 = 5, by8 = 6, by16 = 7 };
    enum class RtcHsePrescaler { by2 = 2, by3 = 3, by4 = 4, by5 = 5, by6 = 6, by7 = 7, by8 = 8, by9 = 9, by10 = 10,
                                 by11 = 11, by12 = 12, by13 = 13, by14 = 14, by15 = 15, by16 = 16, by17 = 17, by18 = 18, by19 = 19,
                                 by21 = 21, by22 = 22, by23 = 23, by24 = 24, by25 = 25, by26 = 26, by27 = 27, by28 = 28, by29 = 29,
                                 by31 = 31 };
    enum class Mco1Prescaler { by1 = 0, by2 = 4, by3 = 5, by4 = 6, by5 = 7 };
    enum class Mco2Prescaler { by1 = 0, by2 = 4, by3 = 5, by4 = 6, by5 = 7 };
    enum class RtcClock { None = 0, LowSpeedExternal = 1, LowSpeedInternal = 2, HighSpeedExternal = 3 };

    ClockControl(System::BaseAddress base, uint32_t externalClock);
    ~ClockControl();

    void addChangeHandler(Callback* changeHandler);
    void removeChangeHandler(Callback* changeHandler);

    bool setSystemClock(uint32_t clock);
    uint32_t clock(Clock clock);
    template<class T>
    void setPrescaler(T prescaler);

    void resetClock();
    void reset();
    Reset::Reason resetReason();

    void enable(Function function, bool inLowPower = true);
    void disable(Function function);

    void enableRtc(RtcClock clock);

private:
    enum
    {
        AHB1 = 0, AHB2, AHB3, APB1 = 4, APB2
    };
    enum
    {
        INTERNAL_CLOCK = 16 * 1000 * 1000,
        CLOCK_WAIT_TIMEOUT = 2000,
    };
    struct RCC
    {
        struct __CR
        {
            uint32_t HSION : 1;
            uint32_t HSIRDY : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t HSITRIM : 5;
            uint32_t HSICAL : 8;
            uint32_t HSEON : 1;
            uint32_t HSERDY : 1;
            uint32_t HSEBYP : 1;
            uint32_t CSSON : 1;
            uint32_t __RESERVED1 : 4;
            uint32_t PLLON : 1;
            uint32_t PLLRDY : 1;
            uint32_t PLLI2SON : 1;
            uint32_t PLLI2SRDY : 1;
            uint32_t __RESERVED2 : 4;
        }   CR;
        struct __PLLCFGR
        {
            uint32_t PLLM : 6;
            uint32_t PLLN : 9;
            uint32_t __RESERVED0 : 1;
            uint32_t PLLP : 2;
            uint32_t __RESERVED1 : 4;
            uint32_t PLLSRC : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t PLLQ : 4;
            uint32_t __RESERVED3 : 4;
        }   PLLCFGR;
        struct __CFGR
        {
            uint32_t SW : 2;
            uint32_t SWS : 2;
            uint32_t HPRE : 4;
            uint32_t __RESERVED0 : 2;
            uint32_t PPRE1 : 3;
            uint32_t PPRE2 : 3;
            uint32_t RTCPRE : 5;
            uint32_t MCO1 : 2;
            uint32_t I2SSCR : 1;
            uint32_t MCO1PRE : 3;
            uint32_t MCO2PRE : 3;
            uint32_t MCO2 : 2;
        }   CFGR;
        struct __CIR
        {
            uint32_t LSIRDYF : 1;
            uint32_t LSERDYF : 1;
            uint32_t HSIRDYF : 1;
            uint32_t HSERDYF : 1;
            uint32_t PLLRDYF : 1;
            uint32_t PLLI2SRDYF : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t CSSF : 1;
            uint32_t LSIRDYE : 1;
            uint32_t LSERDYE : 1;
            uint32_t HSIRDYE : 1;
            uint32_t HSERDYE : 1;
            uint32_t PLLRDYE : 1;
            uint32_t PLLI2SRDYE : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t LSIRDYC : 1;
            uint32_t LSERDYC : 1;
            uint32_t HSIRDYC : 1;
            uint32_t HSERDYC : 1;
            uint32_t PLLRDYC : 1;
            uint32_t PLLI2SRDYC : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t CSSC : 1;
            uint32_t __RESERVED3 : 8;
        }   CIR;
        struct __AHB1RSTR
        {
            uint32_t GPIOARST : 1;
            uint32_t GPIOBRST : 1;
            uint32_t GPIOCRST : 1;
            uint32_t GPIODRST : 1;
            uint32_t GPIOERST : 1;
            uint32_t GPIOFRST : 1;
            uint32_t GPIOGRST : 1;
            uint32_t GPIOHRST : 1;
            uint32_t GPIOIRST : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRCRST : 1;
            uint32_t __RESERVED1 : 8;
            uint32_t DMA1RST : 1;
            uint32_t DMA2RST : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t ETHMACRST : 1;
            uint32_t __RESERVED3 : 3;
            uint32_t OTGHSRST : 1;
            uint32_t __RESERVED4 : 2;
        }   AHB1RSTR;
        struct __AHB2RSTR
        {
            uint32_t DCMIRST : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRYPRST : 1;
            uint32_t HASHRST : 1;
            uint32_t RNGRST : 1;
            uint32_t OTGFSRST : 1;
            uint32_t __RESERVED1 : 24;
        }   AHB2RSTR;
        struct __AHB3RSTR
        {
            uint32_t FSMCRST : 1;
            uint32_t __RESERVED0 : 31;
        }   AHB3RSTR;
        uint32_t __RESERVED0;
        struct __APB1RSTR
        {
            uint32_t TIM2RST : 1;
            uint32_t TIM3RST : 1;
            uint32_t TIM4RST : 1;
            uint32_t TIM5RST : 1;
            uint32_t TIM6RST : 1;
            uint32_t TIM7RST : 1;
            uint32_t TIM12RST : 1;
            uint32_t TIM13RST : 1;
            uint32_t TIM14RST : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t WWDGRST : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t SPI2RST : 1;
            uint32_t SPI3RST : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t USART2RST : 1;
            uint32_t USART3RST : 1;
            uint32_t UART4RST : 1;
            uint32_t UART5RST : 1;
            uint32_t I2C1RST : 1;
            uint32_t I2C2RST : 1;
            uint32_t I2C3RST : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t CAN1RST : 1;
            uint32_t CAN2RST : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t PWRRST : 1;
            uint32_t DACRST : 1;
            uint32_t __RESERVED5 : 2;
        }   APB1RSTR;
        struct __APB2RSTR
        {
            uint32_t TIM1RST : 1;
            uint32_t TIM8RST : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t USART1RST : 1;
            uint32_t USART6RST : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t ADCRST : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t SDIORST : 1;
            uint32_t SPI1RST : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t SYSCFGRST : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t TIM9RST : 1;
            uint32_t TIM10RST : 1;
            uint32_t TIM11RST : 1;
            uint32_t __RESERVED5 : 13;
        }   APB2RSTR;
        uint32_t __RESERVED1;
        uint32_t __RESERVED2;
        uint32_t Enable[8];
        uint32_t LowPowerEnable[8];
/*      Previous definition as bitfield, will now be handled through enum Function
        struct __AHB1ENR
        {
            uint32_t GPIOAEN : 1;
            uint32_t GPIOBEN : 1;
            uint32_t GPIOCEN : 1;
            uint32_t GPIODEN : 1;
            uint32_t GPIOEEN : 1;
            uint32_t GPIOFEN : 1;
            uint32_t GPIOGEN : 1;
            uint32_t GPIOHEN : 1;
            uint32_t GPIOIEN : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRCEN : 1;
            uint32_t __RESERVED1 : 8;
            uint32_t DMA1EN : 1;
            uint32_t DMA2EN : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t ETHMACEN : 1;
            uint32_t __RESERVED3 : 3;
            uint32_t OTGHSEN : 1;
            uint32_t __RESERVED4 : 2;
        }   AHB1ENR;
        struct __AHB2ENR
        {
            uint32_t DCMIEN : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRYPEN : 1;
            uint32_t HASHEN : 1;
            uint32_t RNGEN : 1;
            uint32_t OTGFSEN : 1;
            uint32_t __RESERVED1 : 24;
        }   AHB2ENR;
        struct __AHB3ENR
        {
            uint32_t FSMCEN : 1;
            uint32_t __RESERVED0 : 31;
        }   AHB3ENR;
        uint32_t __RESERVED3;
        struct __APB1ENR
        {
            uint32_t TIM2EN : 1;
            uint32_t TIM3EN : 1;
            uint32_t TIM4EN : 1;
            uint32_t TIM5EN : 1;
            uint32_t TIM6EN : 1;
            uint32_t TIM7EN : 1;
            uint32_t TIM12EN : 1;
            uint32_t TIM13EN : 1;
            uint32_t TIM14EN : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t WWDGEN : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t SPI2EN : 1;
            uint32_t SPI3EN : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t USART2EN : 1;
            uint32_t USART3EN : 1;
            uint32_t UART4EN : 1;
            uint32_t UART5EN : 1;
            uint32_t I2C1EN : 1;
            uint32_t I2C2EN : 1;
            uint32_t I2C3EN : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t CAN1EN : 1;
            uint32_t CAN2EN : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t PWREN : 1;
            uint32_t DACEN : 1;
            uint32_t __RESERVED5 : 2;
        }   APB1ENR;
        struct __APB2ENR
        {
            uint32_t TIM1EN : 1;
            uint32_t TIM8EN : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t USART1EN : 1;
            uint32_t USART6EN : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t ADCEN : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t SDIOEN : 1;
            uint32_t SPI1EN : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t SYSCFGEN : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t TIM9EN : 1;
            uint32_t TIM10EN : 1;
            uint32_t TIM11EN : 1;
            uint32_t __RESERVED5 : 13;
        }   APB2ENR;
        uint32_t __RESERVED4;
        uint32_t __RESERVED5;
        struct __AHB1LPENR
        {
            uint32_t GPIOALPEN : 1;
            uint32_t GPIOBLPEN : 1;
            uint32_t GPIOCLPEN : 1;
            uint32_t GPIODLPEN : 1;
            uint32_t GPIOELPEN : 1;
            uint32_t GPIOFLPEN : 1;
            uint32_t GPIOGLPEN : 1;
            uint32_t GPIOHLPEN : 1;
            uint32_t GPIOILPEN : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRCLPEN : 1;
            uint32_t __RESERVED1 : 8;
            uint32_t DMA1LPEN : 1;
            uint32_t DMA2LPEN : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t ETHMACLPEN : 1;
            uint32_t __RESERVED3 : 3;
            uint32_t OTGHSLPEN : 1;
            uint32_t __RESERVED4 : 2;
        }   AHB1LPENR;
        struct __AHB2LPENR
        {
            uint32_t DCMILPEN : 1;
            uint32_t __RESERVED0 : 3;
            uint32_t CRYPLPEN : 1;
            uint32_t HASHLPEN : 1;
            uint32_t RNGLPEN : 1;
            uint32_t OTGFSLPEN : 1;
            uint32_t __RESERVED1 : 24;
        }   AHB2LPENR;
        struct __AHB3LPENR
        {
            uint32_t FSMCLPEN : 1;
            uint32_t __RESERVED0 : 31;
        }   AHB3LPENR;
        uint32_t __RESERVED6;
        struct __APB1LPENR
        {
            uint32_t TIM2LPEN : 1;
            uint32_t TIM3LPEN : 1;
            uint32_t TIM4LPEN : 1;
            uint32_t TIM5LPEN : 1;
            uint32_t TIM6LPEN : 1;
            uint32_t TIM7LPEN : 1;
            uint32_t TIM12LPEN : 1;
            uint32_t TIM13LPEN : 1;
            uint32_t TIM14LPEN : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t WWDGLPEN : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t SPI2LPEN : 1;
            uint32_t SPI3LPEN : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t USART2LPEN : 1;
            uint32_t USART3LPEN : 1;
            uint32_t UART4LPEN : 1;
            uint32_t UART5LPEN : 1;
            uint32_t I2C1LPEN : 1;
            uint32_t I2C2LPEN : 1;
            uint32_t I2C3LPEN : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t CAN1LPEN : 1;
            uint32_t CAN2LPEN : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t PWRLPEN : 1;
            uint32_t DACLPEN : 1;
            uint32_t __RESERVED5 : 2;
        }   APB1LPENR;
        struct __APB2LPENR
        {
            uint32_t TIM1LPEN : 1;
            uint32_t TIM8LPEN : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t USART1LPEN : 1;
            uint32_t USART6LPEN : 1;
            uint32_t __RESERVED1 : 2;
            uint32_t ADCLPEN : 1;
            uint32_t __RESERVED2 : 2;
            uint32_t SDIOLPEN : 1;
            uint32_t SPI1LPEN : 1;
            uint32_t __RESERVED3 : 1;
            uint32_t SYSCFGLPEN : 1;
            uint32_t __RESERVED4 : 1;
            uint32_t TIM9LPEN : 1;
            uint32_t TIM10LPEN : 1;
            uint32_t TIM11LPEN : 1;
            uint32_t __RESERVED5 : 13;
        }   APB2LPENR;
        uint32_t __RESERVED7;
        uint32_t __RESERVED8;*/
        struct __BDCR
        {
            uint32_t LSEON : 1;
            uint32_t LSERDY : 1;
            uint32_t LSEBYP : 1;
            uint32_t __RESERVED0 : 5;
            uint32_t RTCSEL : 2;
            uint32_t __RESERVED1 : 5;
            uint32_t RTCEN : 1;
            uint32_t BDRST : 1;
            uint32_t __RESERVED2 : 15;
        }   BDCR;
        union __CSR
        {
            struct {
                uint32_t LSION : 1;
                uint32_t LSIRDY : 1;
                uint32_t __RESERVED0 : 22;
                uint32_t RMVF : 1;
                uint32_t BORRSTF : 1;
                uint32_t PINRSTF : 1;
                uint32_t PORRSTF : 1;
                uint32_t SFTRSTF : 1;
                uint32_t IWDGRSTF : 1;
                uint32_t WWDGRSTF : 1;
                uint32_t LPWRRSTF : 1;
            }   CSR;
            uint32_t v;
        }   CSR;
        uint32_t __RESERVED9;
        uint32_t __RESERVEDA;
        struct __SSCGR
        {
            uint32_t MODPER : 13;
            uint32_t INCSTEP : 15;
            uint32_t __RESERVED0 : 2;
            uint32_t SPREADSEL : 1;
            uint32_t SSCGEN : 1;
        }   SSCGR;
        struct __PLLI2SCFGR
        {
            uint32_t __RESERVED0 : 6;
            uint32_t PLLI2SN : 9;
            uint32_t __RESERVED1 : 13;
            uint32_t PLLI2S : 3;
            uint32_t __RESERVED2 : 1;
        }   PLLI2SCFGR;

    };
    volatile RCC* mBase;
    uint32_t mExternalClock;
    std::vector<Callback*> mCallback;

    bool getPllConfig(uint32_t clock, uint32_t& div, uint32_t& mul);
    void resetClock(bool notify);
    void notify(Callback::Reason reason, uint32_t clock);

    uint32_t rtcClock();

    friend void testClockControl();
};

#endif // CLOCKCONTROL_H
