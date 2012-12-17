#ifndef COMMANDS_H
#define COMMANDS_H

#include "CommandInterpreter.h"

struct Command
{
    const char* mName;
    const char* mParam;
    const char* mHelp;
};

class CmdHelp : public CommandInterpreter::Command
{
public:
    CmdHelp() : Command() { }
    virtual bool execute(CommandInterpreter& interpreter, int argc, const char* argv[]);
    virtual unsigned int aliases(const char **&alias) const;
    virtual const char* helpText() const { return "Shows help for all commands, or the one given."; }
private:
    static const char* NAME[];

};

class CmdRead : public CommandInterpreter::Command
{
public:
    CmdRead() : Command() { }
    virtual bool execute(CommandInterpreter& interpreter, int argc, const char* argv[])
    {
        return true;
    }
    virtual unsigned int aliases(const char **&alias) const;
    virtual const char* helpText() const { return "Access memory."; }
private:
    static const char* NAME[];
};

class CmdWrite : public CommandInterpreter::Command
{
public:
    CmdWrite() : Command() { }
    virtual bool execute(CommandInterpreter& interpreter, int argc, const char* argv[])
    {
        return true;
    }
    virtual unsigned int aliases(const char **&alias) const;
    virtual const char* helpText() const { return "Access memory."; }
private:
    static const char* NAME[];
};

#endif // COMMANDS_H
