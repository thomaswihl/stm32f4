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

bool help(CommandInterpreter& ci, const char* line)
{
    return true;
}

bool read(CommandInterpreter& ci, const char* line)
{
    return true;
}
bool write(CommandInterpreter& ci, const char* line)
{
    return true;
}

const CommandInterpreter::Command CommandInterpreter::mCmd[] =
{
    { "help", "command:s:o", "Shows help for all commands or the one given.", help },
    { "hint", nullptr, "Dummy command.", help },
    { "read", "address:u length:u:o", "Read memory of LENGTH [bytes|halfwords|words] from ADDRESS .", read },
    { "write", "address:u value:u length:u:o", "Read memory of LENGTH [bytes|halfwords|words] from ADDRESS .", write },
};

CommandInterpreter::CommandInterpreter(Serial *serial) :
    mSerial(serial),
    mLineLen(0)
{
    strcpy(mPrompt, "# ");
}

void CommandInterpreter::feed()
{
    char c;
    while (mSerial->pop(c))
    {
        switch (c)
        {
        case '\t':
            complete();
            break;
        case '\r':
            mSerial->write("\r\n", 2);
            execute();
            mLineLen = 0;
            printLine();
            break;
        default:
            if (mLineLen < MAX_LINE_LEN) mLine[mLineLen++] = c;
            mSerial->push(c);
        }
    }
}

void CommandInterpreter::start()
{
    printLine();
}

void CommandInterpreter::printLine()
{
    mSerial->write("\r", 1);
    mSerial->write(mPrompt, strlen(mPrompt));
    mSerial->write(mLine, mLineLen);
}

void CommandInterpreter::complete()
{
    const Command* use = nullptr;
    bool found = false;
    Possibilities possible;
    for (const Command& cmd : mCmd)
    {
        if (strncmp(mLine, cmd.mName, mLineLen) == 0)
        {
            if (!found)
            {
                use = &cmd;
                found = true;
            }
            else
            {
                if (use != nullptr)
                {
                    possible.append(use->mName);
                    use = nullptr;
                }
                possible.append(cmd.mName);
            }
        }
    }
    if (found && use != nullptr)
    {
        int useLen = strlen(use->mName);
        strncpy(mLine + mLineLen, use->mName + mLineLen, useLen - mLineLen);
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


