#ifndef INTERRUPTCONTROLLER_H
#define INTERRUPTCONTROLLER_H

#include <cstdint>

class InterruptController
{
public:
    typedef std::uint8_t Index;
    enum class Priority { Prio0, Prio1, Prio2, Prio3, Prio4, Prio5, Prio6, Prio7, Prio8, Prio9, Prio10, Prio11, Prio12, Prio13, Prio14, Prio15 };

    InterruptController(unsigned int base, std::size_t vectorSize);
    ~InterruptController();

    void handle(Index index);
    void setPriotity(Index index, Priority priority);
    void trigger(Index index);

    class Handler
    {
    public:
        Handler() { }
        virtual ~Handler() { }
        virtual void handle(Index index) = 0;
    };

    class Line
    {
    public:
        Line(InterruptController& interruptController, Index index);
        ~Line();
        void setHandler(Handler *handler);
        void enable();
        void disable();
    private:
        InterruptController& mInterruptController;
        Index mIndex;
    };

private:
    struct NVIC
    {
        uint32_t ISER[32];  //0x0000
        uint32_t ICER[32];  //0x0080
        uint32_t ISPR[32];  //0x0100
        uint32_t ICPR[32];  //0x0180
        uint32_t IABR[32];  //0x0200
        uint32_t __RESERVED0[32];  //0x0280
        uint8_t IPR[128];  //0x0300
        uint32_t __RESERVED1[32];  //0x0380
        uint32_t __RESERVED2[640];  //0x0400
        uint16_t STIR;  //0x0e00
    };
    volatile NVIC* mBase;
    InterruptController::Handler** mHandler;
};

#endif // INTERRUPTCONTROLLER_H
