#include "Commands.h"


const char* CmdHelp::NAME[] = { "help", "?" };
const char* CmdRead::NAME[] = { "read", "rb", "rh", "rw" };
const char* CmdWrite::NAME[] = { "write", "wb", "wh", "ww" };


bool CmdHelp::execute(CommandInterpreter &interpreter, int argc, const char *argv[])
{
    for (auto i = interpreter.begin(); i != interpreter.end(); ++i)
    {
        printf("%10s %s\n", i->get()->helpText(), i->get()->helpText());
    }
    return true;
}

unsigned int CmdHelp::aliases(const char **&alias) const
{
    alias = NAME; return sizeof(NAME) / sizeof(NAME[0]);
}

unsigned int CmdRead::aliases(const char **&alias) const
{
    alias = NAME; return sizeof(NAME) / sizeof(NAME[0]);
}

unsigned int CmdWrite::aliases(const char **&alias) const
{
    alias = NAME; return sizeof(NAME) / sizeof(NAME[0]);
}
