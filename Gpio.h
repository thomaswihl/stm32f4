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
    enum class AltFunc { Func0, Func1, Func2, Func3, Func4, Func5, Func6, Func7, Func8, Func9, Func10, Func11, Func12, Func13, Func14, Func15 };
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

    void configInput(Pin index, Speed speed, Pull pull = Pull::None);
    void configOutput(Pin index, Speed speed, OutputType outputType, Pull pull = Pull::None);


private:
    struct GPIO
    {
        uint32_t MODER;
        uint16_t OTYPER;
        uint16_t __RESERVED0;
        uint32_t OSPEEDR;
        uint32_t PUPDR;
        uint16_t IDR;
        uint16_t __RESERVED1;
        uint16_t ODR;
        uint16_t __RESERVED2;
        struct __BSRR
        {
            uint16_t BS;
            uint16_t BR;
        }   BSRR;
        uint32_t LCKR;
        uint32_t AFRL;
        uint32_t AFRH;
    };
    volatile GPIO* mBase;
};

#endif // GPIO_H
