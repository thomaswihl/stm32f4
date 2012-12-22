#include "Commands.h"


char const * const CmdHelp::NAME[] = { "help", "?" };
char const * const CmdHelp::ARGV[] = { "os:command" };

char const * const CmdInfo::NAME[] = { "info" };
char const * const CmdInfo::ARGV[] = { };

char const * const CmdFunc::NAME[] = { "func" };
char const * const CmdFunc::ARGV[] = { "s:function" };

char const * const CmdRead::NAME[] = { "read", "rb", "rh", "rw" };
char const * const CmdRead::ARGV[] = { "u:address", "ou:count" };

char const * const CmdWrite::NAME[] = { "write", "wb", "wh", "ww" };
char const * const CmdWrite::ARGV[] = { "u:address", "u:data" };

char const * const CmdLis::NAME[] = { "lis" };
char const * const CmdLis::ARGV[] = { };


CmdHelp::CmdHelp() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdHelp::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    char const * argCmd = nullptr;
    unsigned int argCmdLen = 0;
    if (argc == 2)
    {
        argCmd = argv[1].value.s;
        argCmdLen = strlen(argv[1].value.s);
    }
    for (auto i : interpreter)
    {
        CommandInterpreter::Command* cmd = i;
        if (argc != 2 || cmd->startsWith(argCmd, argCmdLen) != nullptr)
        {
            interpreter.printAliases(cmd);
            printf(" ");
            interpreter.printArguments(cmd, true);
            printf("\n");
            interpreter.printArguments(cmd, false);
            printf("  %s\n\n", cmd->helpText());
        }
    }
    return true;
}

CmdRead::CmdRead() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdRead::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    unsigned int count = 1;
    if (argc == 3) count = argv[2].value.u;
    switch (argv[0].value.s[1])
    {
    case 'b':
        dump<uint8_t>(reinterpret_cast<uint8_t*>(argv[1].value.u), count);
        break;
    case 'h':
        dump<uint16_t>(reinterpret_cast<uint16_t*>(argv[1].value.u), count);
        break;
    case 'w':
    case 'e':
        dump<uint32_t>(reinterpret_cast<uint32_t*>(argv[1].value.u), count);
        break;
    }
    return true;
}

CmdWrite::CmdWrite() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdWrite::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    return false;
}


CmdInfo::CmdInfo(StmSystem &system) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSystem(system)
{
}

bool CmdInfo::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    mSystem.printInfo();
    return true;
}


CmdFunc::CmdFunc(StmSystem &system) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mSystem(system)
{
}

bool CmdFunc::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    if (strcmp("ns", argv[1].value.s) == 0)
    {
        uint64_t ns = mSystem.ns();
        printf("%llu\n", ns);
    }
    return true;
}


CmdLis::CmdLis(LIS302DL &lis) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mLis(lis)
{
}

bool CmdLis::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    return true;
}
