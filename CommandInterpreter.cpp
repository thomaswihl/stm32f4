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

void CommandInterpreter::add(std::shared_ptr<Command> cmd)
{
    mCmd.push_back(cmd);
}

void CommandInterpreter::start()
{
    printLine();
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
    for (const std::shared_ptr<Command>& cmd : mCmd)
    {
        const char* n = cmd->startsWith(name, len);
        if (n != nullptr)
        {
            possible.append(n);
            if (possible.count() == 1) cmdFound = cmd.get();
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
    static const char* argv[MAX_ARG_LEN];
    argv[0] = mLine;
    unsigned int argc = 1;
    for (unsigned int i = 0; i < mLineLen; ++i)
    {
        if (mLine[i] == ' ')
        {
            mLine[i] = 0;
            argv[argc++] = mLine + i + 1;
        }
    }
    Possibilities possible;
    Command* cmd = findCommand(mLine, mLineLen, possible);
    if (cmd != nullptr) cmd->execute(*this, argc, argv);
}


const char *CommandInterpreter::Command::startsWith(const char *string, unsigned int len) const
{
    const char** alias;
    unsigned int count = aliases(alias);
    if (len == 0) return alias[0];
    for (unsigned int i = 0; i < count; ++i)
    {
        if (strncmp(alias[i], string, len) == 0) return alias[i];
    }
    return nullptr;
}

