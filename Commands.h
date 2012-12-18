#ifndef COMMANDS_H
#define COMMANDS_H

#include "CommandInterpreter.h"

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
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv[]);
    virtual const char* helpText() const { return "Shows help for all commands, or the one given."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

class CmdRead : public CommandInterpreter::Command
{
public:
    CmdRead();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv[]);
    virtual const char* helpText() const { return "Read from memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];

    template<class T>
    void dump(T address, unsigned int count)
    {
        T* p = reinterpret_cast<T*>(address);
        char format[16];
        std::sprintf(format, " %%0%ux", sizeof(T) * 2);
        for (unsigned int i = 0; i < count; ++i)
        {
            if ((i % (32 / sizeof(T))) == 0) printf("\n%08x:", reinterpret_cast<unsigned int>(p));
            printf(format, *p++);
        }
    }
};

class CmdWrite : public CommandInterpreter::Command
{
public:
    CmdWrite();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv[]);
    virtual const char* helpText() const { return "Write to memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

#endif // COMMANDS_H
