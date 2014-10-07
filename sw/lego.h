#ifndef LEGO_H
#define LEGO_H

#include "../StmSystem.h"
#include "../CommandInterpreter.h"

class Lego
{
public:
    Lego();
    void init(StmSystem& sys, CommandInterpreter& interpreter);

private:
    Gpio::Pin* mSs;
};

#endif // LEGO_H
