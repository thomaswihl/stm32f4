#include "Commands.h"

#include <cmath>

char const * const CmdHelp::NAME[] = { "help", "?" };
char const * const CmdHelp::ARGV[] = { "os:command" };

char const * const CmdInfo::NAME[] = { "info" };
char const * const CmdInfo::ARGV[] = { };

char const * const CmdFunc::NAME[] = { "func" };
char const * const CmdFunc::ARGV[] = { "s:function" };

char const * const CmdRead::NAME[] = { "read", "rb", "rh", "rw" };
char const * const CmdRead::ARGV[] = { "Au:address", "Vou:count" };

char const * const CmdWrite::NAME[] = { "write", "wb", "wh", "ww" };
char const * const CmdWrite::ARGV[] = { "Au:address", "Vu:data" };

char const * const CmdLis::NAME[] = { "lis" };
char const * const CmdLis::ARGV[] = { };

char const * const CmdPin::NAME[] = { "pin" };
char const * const CmdPin::ARGV[] = { "Ps:pin", "Vob:value" };


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
    default:
        dump<uint32_t>(reinterpret_cast<uint32_t*>(argv[1].value.u), count);
        break;
    }
    return true;
}

template<class T>
void CmdRead::dump(T* address, unsigned int count)
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

CmdWrite::CmdWrite() : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0]))
{
}

bool CmdWrite::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    switch (argv[0].value.s[1])
    {
    case 'b':
        *reinterpret_cast<uint8_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    case 'h':
        *reinterpret_cast<uint16_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    default:
        *reinterpret_cast<uint32_t*>(argv[1].value.u) = argv[2].value.u;
        break;
    }
    return true;
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
    else
    {
        printf("Unknown function, allowed is: ns\n");
    }
    return true;
}


CmdLis::CmdLis(LIS302DL &lis) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mLis(lis)
{
}

bool CmdLis::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    int x, y, z;
    x = mLis.x();
    y = mLis.y();
    z = mLis.z();
    float a = x * x + y * y + z * z;
    a = std::sqrt(a / 2500);
    printf("%3i %3i %3i = %.2fg\n", x, y, z, a);
    return true;
}


CmdPin::CmdPin(Gpio **gpio, unsigned int gpioCount) : Command(NAME, sizeof(NAME) / sizeof(NAME[0]), ARGV, sizeof(ARGV) / sizeof(ARGV[0])), mGpio(gpio), mGpioCount(gpioCount)
{
}

bool CmdPin::execute(CommandInterpreter &interpreter, int argc, const CommandInterpreter::Argument *argv)
{
    char c = argv[1].value.s[0];
    unsigned int index = c - 'A';
    if (index > mGpioCount) index = c - 'a';
    if (index > mGpioCount)
    {
        printf("Invalid GPIO, GPIO A-%c (or a-%c) is allowed.\n", 'A' + mGpioCount - 1, 'a' + mGpioCount - 1);
        return false;
    }
    char* end;
    unsigned int pin = strtoul(argv[1].value.s +1, &end, 10);
    if (*end != 0 || pin > 15)
    {
        printf("Invalid index, GPIO has only 16 pins, so 0-15 is allowed.\n");
        return false;
    }
    if (argc == 2)
    {
        printf("GPIO %c%i = %c\n", 'A' + index, pin, mGpio[index]->get(static_cast<Gpio::Index>(pin)) ? '1' : '0');
    }
    else
    {
        if (argv[2].value.b) mGpio[index]->set(static_cast<Gpio::Index>(pin));
        else mGpio[index]->reset(static_cast<Gpio::Index>(pin));
    }
    return true;
}
