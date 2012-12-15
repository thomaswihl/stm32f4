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

#ifndef GPIO_H
#define GPIO_H

#include <cstdint>

class Gpio
{
public:
    Gpio(unsigned int base);
    ~Gpio();

    enum class Pin { Pin0, Pin1, Pin2, Pin3, Pin4, Pin5, Pin6, Pin7, Pin8, Pin9, Pin10, Pin11, Pin12, Pin13, Pin14, Pin15 };
    enum class OutputType { PushPull = 0, OpenDrain = 1 };
    enum class Speed { Low = 0, Medium = 1, Fast = 2, High = 3 };
    enum class Pull { None = 0, Up = 1, Down = 2 };
    enum class AltFunc
    {
        GPIO = 0,
        TIM1 = 1,
        TIM2 = 1,
        TIM3 = 2,
        TIM4 = 2,
        TIM5 = 2,
        TIM8 = 3,
        TIM9 = 3,
        TIM10 = 3,
        TIM11 = 3,
        I2C1 = 4,
        I2C2 = 4,
        I2C3 = 4,
        SPI1 = 5,
        SPI2 = 5,
        SPI3 = 6,
        USART1 = 7,
        USART2 = 7,
        USART3 = 7,
        UART4 = 8,
        UART5 = 8,
        USART6 = 8,
        CAN1 = 9,
        CAN2 = 9,
        TIM12 = 9,
        TIM13 = 9,
        TIM14 = 9,
        OTG_FS = 10,
        OTG_HS = 10,
        ETH = 11,
        FSMC = 12,
        SDIO = 12,
        OTG_HS_FS = 12,
        DCMI = 13,
        EVENTOUT = 15
    };
    enum class Mode { Input = 0, Output = 1, Alternate = 2, Analog = 3 };

    bool get(Pin index);
    uint16_t get();
    void set(Pin index);
    void set(uint16_t indices);
    void setValue(uint16_t value);
    void reset(Pin index);
    void reset(uint16_t indices);

    void setMode(Pin index, Mode mode);
    void setOutputType(Pin index, OutputType outputType);
    void setSpeed(Pin index, Speed speed);
    void setPull(Pin index, Pull pull);
    void setAlternate(Pin index, AltFunc altFunc);

    void configInput(Pin index, Pull pull = Pull::None);
    void configOutput(Pin index, OutputType outputType, Pull pull = Pull::None, Speed speed = Speed::Medium);


private:
    struct GPIO
    {
        uint32_t MODER;
        uint32_t OTYPER;
        uint32_t OSPEEDR;
        uint32_t PUPDR;
        uint32_t IDR;
        uint32_t ODR;
        struct __BSRR
        {
            uint32_t BS : 16;
            uint32_t BR : 16;
        }   BSRR;
        uint32_t LCKR;
        uint32_t AFRL;
        uint32_t AFRH;
    };
    volatile GPIO* mBase;
};

#endif // GPIO_H
