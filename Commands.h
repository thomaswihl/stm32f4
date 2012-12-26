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
    void dump(T* address, unsigned int count)
    {
        char format[16];
        std::sprintf(format, " %%0%ux", sizeof(T) * 2);
        T* p = address;
        for (unsigned int i = 0; i < count; ++i)
        {
            if ((i % (32 / sizeof(T))) == 0)
            {
                if (i != 0) printf("\n");
                printf("%08x:", reinterpret_cast<unsigned int>(p));
            }
            printf(format, *p++);
        }
        printf("\n");
    }
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


#endif // COMMANDS_H
