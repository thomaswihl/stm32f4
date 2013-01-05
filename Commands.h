#ifndef COMMANDS_H
#define COMMANDS_H

#include "CommandInterpreter.h"
#include "StmSystem.h"
#include "hw/lis302dl.h"

#include <cstdio>

struct Command
{
    const char* mName;
    const char* mParam;
    const char* mHelp;
};

class CmdHelp : public CommandInterpreter::Command
{
public:
    CmdHelp();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Shows help for all commands, or the one given."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

class CmdInfo : public CommandInterpreter::Command
{
public:
    CmdInfo(StmSystem& system);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Shows system information."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    StmSystem& mSystem;
};

class CmdFunc : public CommandInterpreter::Command
{
public:
    CmdFunc(StmSystem& system);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Execute a function and show the result."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    StmSystem& mSystem;
};

class CmdRead : public CommandInterpreter::Command
{
public:
    CmdRead();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Read from memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];

    template<class T>
    void dump(T* address, unsigned int count);
};

class CmdWrite : public CommandInterpreter::Command
{
public:
    CmdWrite();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Write to memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

class CmdLis : public CommandInterpreter::Command
{
public:
    CmdLis(LIS302DL& lis);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Show LIS302DL info."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    LIS302DL& mLis;
};

class CmdPin : public CommandInterpreter::Command
{
public:
    CmdPin(Gpio** gpio, unsigned int gpioCount);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Read or write GPIO pin."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    Gpio** mGpio;
    unsigned int mGpioCount;
};


#endif // COMMANDS_H
