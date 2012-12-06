#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <cstdint>

class Interrupt
{
public:
    typedef std::uint8_t Index;
    class Handler
    {
    public:
        Handler() { }
        virtual ~Handler() { }
        virtual void handle(Index index) = 0;
    };

    Interrupt(unsigned int base, std::size_t vectorSize);
    ~Interrupt();

    void set(Index index, Handler *handler);
    void handle(Index index);
private:
    struct EXTI
    {
        uint32_t IMR : 23;
        uint32_t : 0;
        uint32_t EMR : 23;
        uint32_t : 0;
    };

    volatile EXTI* mBase;
    Handler** mHandler;

};

#endif // INTERRUPT_H
