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
    mLineLen(0)
{
    strcpy(mPrompt, "# ");
}

void CommandInterpreter::feed()
{
    char c;
    while (mSystem.mDebug.pop(c))
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
            if (mLineLen > 0)
            {
                --mLineLen;
                mSystem.mDebug.push(c);
                mSystem.mDebug.push(' ');
                mSystem.mDebug.push(c);
            }
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

void CommandInterpreter::complete()
{
    const char* useName = nullptr;
    bool found = false;
    Possibilities possible;
    for (const std::shared_ptr<Command>& cmd : mCmd)
    {
        const char* name = cmd->startsWith(mLine, mLineLen);
        if (name != nullptr)
        {
            if (!found)
            {
                useName = name;
                found = true;
            }
            else
            {
                if (useName != nullptr)
                {
                    possible.append(useName);
                    useName = nullptr;
                }
                possible.append(name);
            }
        }
    }
    if (found && useName != nullptr)
    {
        int useLen = strlen(useName);
        strncpy(mLine + mLineLen, useName + mLineLen, useLen - mLineLen);
        mLineLen = useLen;
        mLine[mLineLen++] = ' ';
        printLine();
    }
    else if (found)
    {
        std::printf("\n%s\n", possible.string());
        printLine();
    }
}

void CommandInterpreter::execute()
{
}


const char *CommandInterpreter::Command::startsWith(const char *string, unsigned int len) const
{
    const char** alias;
    unsigned int count = aliases(alias);
    for (unsigned int i = 0; i < count; ++i)
    {
        if (strncmp(alias[i], string, len) == 0) return alias[i];
    }
    return nullptr;
}

