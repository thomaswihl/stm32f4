#include "Commands.h"


const char* CmdHelp::NAME[] = { "help", "?" };
const char* CmdRead::NAME[] = { "read", "rb", "rh", "rw" };
const char* CmdWrite::NAME[] = { "write", "wb", "wh", "ww" };


bool CmdHelp::execute(CommandInterpreter &interpreter, int argc, const char *argv[])
{
    for (auto cmd = interpreter.begin(); cmd != interpreter.end(); ++cmd)
    {
        if (argc != 2 || cmd->get()->startsWith(argv[1], strlen(argv[1])) != nullptr)
        {
            const char** alias;
            unsigned int count = cmd->get()->aliases(alias);
            for (unsigned int i = 0; i < count; ++i) printf("%s %c ", alias[i], (i < (count -1)) ? '|' : ' ' );
            printf("\n  %s\n", cmd->get()->helpText());
        }
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
