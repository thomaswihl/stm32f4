/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CommandInterpreter.h"

#include <cstring>
#include <algorithm>

CommandInterpreter::CommandInterpreter(StmSystem& system) :
    mSystem(system),
    mLineLen(0),
    mState(State::Input)
{
    strcpy(mPrompt, "# ");
}

CommandInterpreter::~CommandInterpreter()
{
    for (auto cmd : mCmd) delete cmd;
}

void CommandInterpreter::feed()
{
    char c;
    while (mSystem.mDebug.pop(c))
    {
        if (mState == State::Input)
        {
            switch (c)
            {
            case '\t':
                complete();
                break;
            case '\r':
                mSystem.mDebug.write("\r\n", 2);
                execute();
                mLineLen = 0;
                printLine();
                break;
            case 8:
            case 127:
                if (mLineLen > 0)
                {
                    --mLineLen;
                    mSystem.mDebug.push(8);
                    mSystem.mDebug.push(' ');
                    mSystem.mDebug.push(8);
                }
                break;
            case 4:     // Ctrl+D
                mState = State::Debug;
                printf("\nEntering DEBUG mode\n");
                mLineLen = 0;
                printLine();
                break;
            case 18:    // Ctrl+R
                mSystem.mRcc.resetClock();
                mSystem.printInfo();
                break;
            default:
                if (mLineLen < MAX_LINE_LEN) mLine[mLineLen++] = c;
                mSystem.mDebug.push(c);
                break;
            }
        }
        else if (mState == State::Debug)
        {
            if (c != 4)
            {
                printf("received 0x%02x (%u), press Ctrl+D to exit debug mode.\n", c, c);
                printLine();
            }
            else
            {
                printf("\nLeaving DEBUG mode\n");
                mState = State::Input;
                printLine();
            }
        }
    }
}

void CommandInterpreter::add(Command* cmd)
{
    mCmd.push_back(cmd);
}

void CommandInterpreter::start()
{
    printLine();
}

void CommandInterpreter::printUsage(CommandInterpreter::Command *cmd)
{
    printf("Usage: ");
    printAliases(cmd);
    printf(" ");
    printArguments(cmd, true);
    printf("\n");
}

void CommandInterpreter::printArguments(CommandInterpreter::Command *cmd, bool summary)
{
    unsigned int count = cmd->argumentCount() - 1;
    for (unsigned int i = 0; i <= count; ++i)
    {
        if (summary)
        {
            printf("%s ", cmd->argument(i));
        }
        else
        {
            printf("%10s: %s%s", cmd->argument(i), "o", "s");
        }
    }
}

void CommandInterpreter::printAliases(CommandInterpreter::Command *cmd)
{
    unsigned int count = cmd->aliasCount() - 1;
    for (unsigned int i = 0; i <= count; ++i)
    {
        printf("%s", cmd->alias(i));
        if (i < count) printf(" | ");
    }
}

void CommandInterpreter::printLine()
{
    mSystem.mDebug.push('\r');
    mSystem.mDebug.write(mPrompt, strlen(mPrompt));
    mSystem.mDebug.write(mLine, mLineLen);
}

CommandInterpreter::Command *CommandInterpreter::findCommand(const char *name, unsigned int len, CommandInterpreter::Possibilities &possible)
{
    Command* cmdFound;
    for (Command*& cmd : mCmd)
    {
        const char* n = cmd->startsWith(name, len);
        if (n != nullptr)
        {
            possible.append(n);
            if (possible.count() == 1) cmdFound = cmd;
        }
    }
    if (possible.count() == 1) return cmdFound;
    return nullptr;
}

void CommandInterpreter::complete()
{
    Possibilities possible;
    findCommand(mLine, mLineLen, possible);
    const char* useName = possible.string();
    if (possible.count() == 1)
    {
        int useLen = strlen(useName);
        strncpy(mLine + mLineLen, useName + mLineLen, useLen - mLineLen);
        mLineLen = useLen;
        printLine();
    }
    else if (possible.count() > 1)
    {
        std::printf("\n%s\n", useName);
        printLine();
    }
}

void CommandInterpreter::execute()
{
    static Argument* argv[MAX_ARG_LEN];
    argv[0]->value.s = mLine;
    argv[0]->type = 's';
    unsigned int argc = 1;
    Possibilities possible;
    Command* cmd = findCommand(mLine, mLineLen, possible);
    if (cmd == nullptr)
    {
        mLine[mLineLen] = 0;
        printf("Unknown command: %s, try help.\n", mLine);
        return;
    }
    unsigned int maxArgc = cmd->argumentCount();
    unsigned int minArgc = 0;
    for (unsigned int i = 0; i < mLineLen; ++i)
    {
        if (mLine[i] == ' ')
        {
            mLine[i] = 0;
            argv[argc]->value.s = mLine + i + 1;
            argv[argc]->type = 's';
            ++argc;
            if (argc > maxArgc)
            {
                printUsage(cmd);
                return;
            }
        }
    }
    if (argc < minArgc)
    {
        printUsage(cmd);
        return;
    }
    if (cmd != nullptr) cmd->execute(*this, argc, const_cast<const Argument**>(argv));
}


const char *CommandInterpreter::Command::startsWith(const char *string, unsigned int len) const
{
    if (len == 0) return mAlias[0];
    for (unsigned int i = 0; i < mAliasCount; ++i)
    {
        if (strncmp(mAlias[i], string, len) == 0) return mAlias[i];
    }
    return nullptr;
}

