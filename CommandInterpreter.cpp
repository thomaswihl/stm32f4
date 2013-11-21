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
#include "sw/images.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <strings.h>

CommandInterpreter::CommandInterpreter(StmSystem& system) :
    mSystem(system),
    mTickEvent(*this),
    mLineLen(0),
    mState(State::Input),
    mCommandTime(false),
    mReadChar(0),
    mCharReceived(*this),
    mHistory(64),
    mHistoryIndex(0),
    mEscapeLen(0),
    mFirstSpace(0),
    mFbIndex(0),
    mFbIndexOffset(1)
{
    strcpy(mPrompt, "# ");
    mDisplay[0] = mDisplay[1] = nullptr;
}

CommandInterpreter::~CommandInterpreter()
{
    for (auto cmd : mCmd) delete cmd;
}

void CommandInterpreter::feed()
{
    if (mState == State::Input)
    {
        switch (mReadChar)
        {
        case '\t':
            complete();
            break;
        case '\r':
        case '\n':
            mSystem.mDebug.write("\r\n", 2);
            if (mHistoryIndex != 0)
            {
                strcpy(mLine, mHistory[mHistoryIndex]);
                mLineLen = strlen(mHistory[mHistoryIndex]);
                mHistoryIndex = 0;
            }
            execute();
            mLineLen = 0;
            mFirstSpace = 0;
            printLine();
            break;
        case 8:
        case 127:
            if (mLineLen > 0)
            {
                --mLineLen;
                printLine();
            }
            break;
        case 4:     // Ctrl+D
            mState = State::Debug;
            printf("\nEntering DEBUG mode\n");
            mLineLen = 0;
            printLine();
            break;
        case 18:    // Ctrl+R
            printf("\nResetting clock\n");
            mSystem.mRcc.resetClock();
            mSystem.printInfo();
            printLine();
            break;
        case 20:    // Ctrl+T
            mCommandTime = !mCommandTime;
            printf("\nCommand time %s.\n", mCommandTime ? "on" : "off");
            printLine();
            break;
        case 27:
            mEscapeLen = 0;
            mEscape[mEscapeLen++] = mReadChar;
            mState = State::Escape;
            break;
        default:
            if (mReadChar == ' ')
            {
                if (mLineLen == 0) break;
                else if (mFirstSpace == 0) mFirstSpace = mLineLen;
            }
            if (mLineLen < MAX_LINE_LEN) mLine[mLineLen++] = mReadChar;
            mSystem.mDebug.write(&mReadChar, 1);
            break;
        }
    }
    else if (mState == State::Debug)
    {
        if (mReadChar != 4)
        {
            printf("received 0x%02x (%u), press Ctrl+D to exit debug mode.\n", mReadChar, mReadChar);
            printLine();
        }
        else
        {
            printf("\nLeaving DEBUG mode\n");
            mState = State::Input;
            printLine();
        }
    }
    else if (mState == State::Escape)
    {
        mEscape[mEscapeLen++] = mReadChar;
        if (mEscapeLen == 2 && mReadChar >= 0x40 && mReadChar <= 0x5f)
        {
            // [
        }
        else
        {
            if (mReadChar >= 0x40 && mReadChar <= 0x7e)
            {
                bool unknown = true;
                if (mEscapeLen == 3 && mEscape[0] == 0x1b && mEscape[1] == 0x5b)
                {
                    switch (mReadChar)
                    {
                    case 0x41:  // Curser Up
                        unknown = false;
                        if (mHistoryIndex > -static_cast<int>(mHistory.used())) --mHistoryIndex;
                        printLine();
                        break;
                    case 0x42:  // Curser Down
                        unknown = false;
                        if (mHistoryIndex < 0) ++mHistoryIndex;
                        printLine();
                        break;
                    case 0x44:  // Curser Left
                        break;
                    case 0x43:  // Curser Right
                        break;
                    case 0x35:  // Page Up
                        break;
                    case 0x36:  // Page Down
                        break;
                    case 0x48:  // Pos1
                        break;
                    case 0x46:  // End
                        break;
                    case 0x32:  // Insert
                        break;
                    case 0x33:  // Del
                        break;
                    }
                }
                mState = State::Input;
                if (unknown)
                {
                    printf("\nESC: ");
                    for (unsigned int i = 0; i < mEscapeLen; ++i) printf("%02x ", mEscape[i]);
                    printf("\n");
                    printLine();
                }
                mState = State::Input;
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
    mSystem.mDebug.read(&mReadChar, 1, &mCharReceived);
    mSystem.mSysTick.setEvent(&mTickEvent);
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
    if (cmd->argumentCount() == 0) return;
    Argument arg;
    unsigned int count = cmd->argumentCount() - 1;
    for (unsigned int i = 0; i <= count; ++i)
    {
        arg.name = cmd->argument(i);
        arg.value.s = nullptr;
        parseArgument(arg);
        if (summary)
        {
            if (!arg.optional) printf("%s ", arg.name);
            else printf("<%s> ", arg.name);
        }
        else
        {
            printf("%10s: %s", arg.name, arg.optional ? "optional " : "");
            switch (arg.type)
            {
            case Argument::Type::String: printf("string\n"); break;
            case Argument::Type::Int: printf("int\n"); break;
            case Argument::Type::UnsignedInt: printf("unsigned int\n"); break;
            case Argument::Type::Bool: printf("bool\n"); break;
            case Argument::Type::Unknown: printf("unknown\n"); break;
            }
        }
    }
}

void CommandInterpreter::printAliases(CommandInterpreter::Command *cmd)
{
    unsigned int count = cmd->aliasCount() - 1;
    if (count > 0) printf("[");
    for (unsigned int i = 0; i <= count; ++i)
    {
        printf("%s", cmd->alias(i));
        if (i < count) printf("|");
    }
    if (count > 0) printf("]");
}

bool CommandInterpreter::parseArgument(CommandInterpreter::Argument &argument)
{
    const char* string = argument.name;
    argument.name = strchr(string, ':');
    if (argument.name == nullptr)
    {
        argument.name = string;
        argument.optional = false;
        argument.type = Argument::Type::String;
        return true;
    }
    argument.optional = false;
    argument.type = Argument::Type::Unknown;
    argument.enhancedType = Argument::EnhancedType::Unknown;
    char* end;
    for (; string < argument.name; ++string)
    {
        switch (*string)
        {
        case 'u':
            if (argument.type != Argument::Type::Unknown) return false;
            if (argument.value.s != nullptr)
            {
                argument.value.u = strtoul(argument.value.s, &end, 0);
                if (*end != 0) return false;
            }
            argument.type = Argument::Type::UnsignedInt;
            break;
        case 'i':
            if (argument.type != Argument::Type::Unknown) return false;
            if (argument.value.s != nullptr)
            {
                argument.value.i = strtol(argument.value.s, &end, 0);
                if (*end != 0) return false;
            }
            argument.type = Argument::Type::Int;
            break;
        case 's':
            if (argument.type != Argument::Type::Unknown) return false;
            argument.type = Argument::Type::String;
            break;
        case 'b':
            if (argument.type != Argument::Type::Unknown) return false;
            if (argument.value.s != nullptr)
            {
                if (strcasecmp(argument.value.s, "on") == 0) argument.value.b = true;
                else if (strcasecmp(argument.value.s, "off") == 0) argument.value.b = false;
                else if (strcmp(argument.value.s, "1") == 0) argument.value.b = true;
                else if (strcmp(argument.value.s, "0") == 0) argument.value.b = false;
                else return false;
            }
            argument.type = Argument::Type::Bool;
            break;
        case 'o':
            argument.optional = true;
            break;
        case 'A':
            argument.enhancedType = Argument::EnhancedType::Address;
            break;
        case 'P':
            argument.enhancedType = Argument::EnhancedType::Pin;
            break;
        case 'V':
            argument.enhancedType = Argument::EnhancedType::Value;
            break;
        default:
            return false;
        }
    }
    if (argument.type == Argument::Type::Unknown) return false;
    ++argument.name;
    return true;
}

void CommandInterpreter::eventCallback(System::Event* event)
{
    if (event == &mCharReceived)
    {
        if (event->result() != System::Event::Result::Success)
        {
            static const char* const OVERRUN_ERROR = "Overrun";
            static const char* const PARITY_ERROR = "Parity";
            static const char* const FRAMING_ERROR = "Framing";
            const char* s = "";
            switch (event->result())
            {
            case System::Event::Result::OverrunError: s = OVERRUN_ERROR; break;
            case System::Event::Result::ParityError: s = PARITY_ERROR; break;
            case System::Event::Result::FramingError: s = FRAMING_ERROR; break;
            default: break;
            }
            printf("\nERROR: %s\n", s);
            printLine();
        }
        feed();
        mSystem.mDebug.read(&mReadChar, 1, &mCharReceived);
    }
    else if (event == &mTickEvent)
    {
        const uint8_t* images[] = { IMG_01, IMG_02, IMG_03, IMG_04, IMG_05, IMG_06, IMG_07, IMG_08, IMG_09, IMG_10 };
        if (mDisplay[0] != nullptr)
        {
            mDisplay[0]->sendData(images[mFbIndex]);
            if (mDisplay[1] != nullptr) mDisplay[1]->sendData(images[mFbIndex]);
            mFbIndex += mFbIndexOffset;
            if (mFbIndex <= 0)
            {
                mFbIndexOffset = 1;
            }
            else if (mFbIndex >= static_cast<int>(sizeof(images) / sizeof(images[0]) - 3))
            {
                mFbIndexOffset = -1;
            }
        }
//        static unsigned int ps = -1;
//        unsigned int s = mSystem.mSysTick.ticks() * mSystem.mSysTick.interval() / 1000;
//        if (s != ps)
//        {
//            ps = s;
//            uint32_t te = mSystem.timeInEvent() / 1000000;
//            uint32_t ti = mSystem.timeInInterrupt() / 1000000;
//            uint32_t ic = mSystem.interruptCount();
//            uint32_t ec = mSystem.eventCount();
//            printf("\x1b[s\x1b[;H%4u:%02u:%02u, %10lu.%03lus in %10lu Events, %10lu.%03lus in %10lu IRQs\x1b[K\x1b[u", s / 3600, (s / 60) % 60, s % 60,
//                   te / 1000, te % 1000, ec,
//                   ti / 1000, ti % 1000, ic);
//            fflush(nullptr);
//            System::instance()->debugMsg(((s % 64) < 32) ? "#" : ".", 1);
//        }
    }
}

void CommandInterpreter::printLine()
{
    mSystem.mDebug.write("\r", 1);
    if (mHistoryIndex == 0)
    {
        mSystem.mDebug.write(mPrompt, strlen(mPrompt));
        mSystem.mDebug.write(mLine, mLineLen);
    }
    else
    {
        char buf[16];
        int len = sprintf(buf, "(%i) ", mHistoryIndex);
        mSystem.mDebug.write(buf, len);
        char* p = mHistory[mHistoryIndex];
        mSystem.mDebug.write(p, strlen(p));
    }
    static const char* CLEAR_LINE = "\x1b[0K";
    mSystem.mDebug.write(CLEAR_LINE, 4);
}

CommandInterpreter::Command *CommandInterpreter::findCommand(const char *name, unsigned int len, CommandInterpreter::Possibilities &possible)
{
    Command* cmdFound = nullptr;
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
    if (mFirstSpace == 0)
    {
        Possibilities possible;
        findCommand(mLine, mLineLen, possible);
        const char* useName = possible.string();
        if (possible.count() == 1)
        {
            int useLen = strlen(useName);
            strncpy(mLine + mLineLen, useName + mLineLen, useLen - mLineLen);
            mLineLen = useLen;
            mFirstSpace = mLineLen - 1;
            printLine();
        }
        else if (possible.count() > 1)
        {
            std::printf("\n%s\n", useName);
            printLine();
        }
    }
    else
    {
        Possibilities possible;
        Command* cmd = findCommand(mLine, mFirstSpace, possible);
        if (cmd != nullptr)
        {
            unsigned int argc = findArguments(false);
            if (argc >= 2)
            {
                Argument arg;
                arg.name = cmd->argument(argc - 2);
                arg.value.s = nullptr;
                parseArgument(arg);
                if (arg.enhancedType == Argument::EnhancedType::Address)
                {
                    unsigned int i = mLineLen - 1;
                    bool hex = false;
                    while (i > 0 && mLine[i] != ' ')
                    {
                        if (mLine[i] == 'x' || mLine[i] == 'X')
                        {
                            hex = true;
                            break;
                        }
                        --i;
                    }
                    if (hex)
                    {
                        for (unsigned int j = mLineLen - i; j < 9; ++j) mLine[mLineLen++] = '0';
                        mLine[mLineLen++] = ' ';
                        printLine();
                    }
                }
            }
        }
    }
}

unsigned int CommandInterpreter::findArguments(bool splitArgs)
{
    mArguments[0].value.s = mLine;
    mArguments[0].type = Argument::Type::String;
    unsigned int argc = 1;
    char split = ' ';
    int argStart = 0, prevArgStart = 0;
    for (unsigned int i = 0; i < mLineLen; ++i)
    {
        prevArgStart = argStart;
        if (mLine[i] == split)
        {
            if(splitArgs) mLine[i] = 0;
            split = ' ';
            ++argStart;
        }
        else if (split == ' ' && (mLine[i] == '"' || mLine[i] == '\''))
        {
            if(splitArgs) mLine[i] = 0;
            split = mLine[i];
            ++argStart;
        }
        if (argStart > 0 && argStart == prevArgStart)
        {
            mArguments[argc].value.s = mLine + i;
            mArguments[argc].type = Argument::Type::String;
            ++argc;
            if (argc >= MAX_ARG_LEN) break;
            argStart = 0;
        }
    }
    return argc;
}

void CommandInterpreter::execute()
{
    mLine[mLineLen] = 0;
    addToHistory(mLine);
    // first we split our line into seperate strings (by replacing whitespace with binary 0)
    unsigned int argc = findArguments(true);
    if (mArguments[0].value.s[0] == 0) return;
    Possibilities possible;
    Command* cmd = findCommand(mArguments[0].value.s, strlen(mArguments[0].value.s), possible);
    if (cmd == nullptr)
    {
        printf("Unknown command: %s", mArguments[0].value.s);
//        const char* p = mArguments[0].value.s;
//        while (*p != 0)
//        {
//            printf(" %02x", *p++);
//        }
        printf(", try help.\n");
        return;
    }
    unsigned int maxArgc = std::min(static_cast<unsigned int>(MAX_ARG_LEN), cmd->argumentCount() + 1);
    unsigned int minArgc = 1;
    bool failed = false;
    // If everything is fine so far we parse our arguments and check if they are valid.
    for (unsigned int i = 1; i < argc; ++i)
    {
        mArguments[i].name = cmd->argument(i - 1);
        //printf("Parsing %s/%s: ", argv[i].name, argv[i].value.s);
        if (!parseArgument(mArguments[i]))
        {
            failed = true;
            break;
        }
        //printf("%u\n", argv[i].value.u);
        if (!mArguments[i].optional) ++minArgc;
    }
    // If we fond a problem print a usage message.
    if (failed || argc < minArgc || argc > maxArgc)
    {
        if (argc > maxArgc || argc < minArgc) printf("Wrong argument count (%i != [%i, %i]).\n", argc - 1, minArgc - 1, maxArgc - 1);
        printUsage(cmd);
        return;
    }
    // Everything is fine, execute it.
    if (cmd != nullptr)
    {
        uint64_t start = System::instance()->ns();
        cmd->execute(*this, argc, mArguments);
        if (mCommandTime)
        {
            uint64_t delta = System::instance()->ns() - start;
            printf("Command took %u.%06us\n", static_cast<unsigned int>(delta / 1000000000), static_cast<unsigned int>((delta / 1000) % 1000000));
        }
    }
}

void CommandInterpreter::addToHistory(const char *line)
{
    unsigned int len = strlen(line) + 1;
    if (len == 1) return;
    char* copy = new char[len];
    memcpy(copy, line, len);
    if (mHistory.free() == 0)
    {
        char* del;
        if (mHistory.pop(del))
        {
            delete[] del;
        }
    }
    mHistory.push(copy);
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

