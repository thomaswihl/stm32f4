#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

class System
{
public:
    class IrqHandler
    {
    public:
        IrqHandler() { }
        virtual ~IrqHandler() { }
        virtual void irq(int index) = 0;
    };

    enum class TrapIndex
    {
        NMI = 2,
        HardFault = 3,
        MemManage = 4,
        BusFault = 5,
        UsageFault = 6,
        SVCall = 11,
        DebugMonitor = 12,
        PendSV = 14,
        SysTick = 15,
        __COUNT
    };

    enum class InterruptIndex
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
    static void preInit();
    void init();
    uint32_t getCoreClock();
    bool handleIsr(unsigned int index) const;
    static System* instance();
    void setInterrupt(InterruptIndex index, IrqHandler* handler);
protected:
    System();
    ~System();

    class Trap : public IrqHandler
    {
    public:
        Trap(const char* name);
        virtual ~Trap() { }
        virtual void irq(int index);
    private:
        const char* mName;
    };

    void setTrap(TrapIndex index, IrqHandler* handler);

private:
    static System* mSystem;
    IrqHandler* mTrap[static_cast<int>(TrapIndex::__COUNT)];
    IrqHandler* mInterrupt[static_cast<int>(InterruptIndex::__COUNT)];

    Trap mNmi;
    Trap mHardFault;
    Trap mMemManage;
    Trap mBusFault;
    Trap mUsageFault;
    Trap mSVCall;
    Trap mDebugMonitor;
    Trap mPendSV;
    Trap mSysTick;

    static void setSysClock();
    uint32_t getCoreClockPll();
    static void initUart();

};

#endif
