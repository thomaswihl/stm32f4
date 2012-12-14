#include "CommandInterpreter.h"

CommandInterpreter::Command CommandInterpreter::mCmd[] =
{
    { "help", { "command:str:o" } }
};

CommandInterpreter::CommandInterpreter()
{
}
