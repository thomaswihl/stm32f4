#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "System.h"

class ClockControl
{
public:
    ClockControl(System::BaseAddress base, uint32_t externalClock);
    ~ClockControl();

    bool setSystemClock(uint32_t clock);
private:
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
        uint32_t __RESERVED8;
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
        struct __CSR
        {
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

    bool getPllConfig(uint32_t clock, uint32_t& div, uint32_t& mul);

    friend int testClockControl();
};

#endif // CLOCKCONTROL_H
