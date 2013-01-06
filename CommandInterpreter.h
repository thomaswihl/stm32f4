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

#ifndef COMMANDINTERPRETER_H
#define COMMANDINTERPRETER_H

#include "Serial.h"
#include "StmSystem.h"
#include "CircularBuffer.h"

#include <vector>

class CommandInterpreter : public System::Event::Callback
{
public:
    struct Argument
    {
        enum class Type { Unknown, String, UnsignedInt, Int, Bool };
        union __value
        {
            unsigned int u;
            int i;
            char const * s;
            bool b;
        }   value;
        Type type;
        bool optional;
        const char* name;
    };
    struct Command
    {
    public:
        Command(char const *const * alias, unsigned int aliasCount, char const *const * argument, unsigned int argumentCount) : mAlias(alias), mAliasCount(aliasCount), mArgument(argument), mArgumentCount(argumentCount) { }

        const char* startsWith(const char* string, unsigned int len) const;
        char const *const alias(unsigned int i) { return (i < mAliasCount) ? mAlias[i] : nullptr; }
        unsigned int aliasCount() { return mAliasCount; }
        char const *const argument(unsigned int i) { return (i < mArgumentCount) ? mArgument[i] : nullptr; }
        unsigned int argumentCount() { return mArgumentCount; }

        virtual bool execute(CommandInterpreter& interpreter, int argc, const Argument* argv) = 0;
        virtual const char* helpText() const = 0;

    protected:
        char const *const * mAlias;
        unsigned int mAliasCount;
        char const *const * mArgument;
        unsigned int mArgumentCount;
    };

    typedef std::vector<Command*>::iterator iterator;
    typedef std::vector<Command*>::const_iterator const_iterator;
    iterator begin() { return mCmd.begin(); }
    iterator end() { return mCmd.end(); }

    CommandInterpreter(StmSystem& system);
    ~CommandInterpreter();

    void feed();
    void add(Command *cmd);
    void start();
    void printUsage(Command* cmd);
    void printArguments(Command* cmd, bool summary);
    void printAliases(Command* cmd);
    bool parseArgument(Argument& argument);
protected:
    virtual void eventCallback(System::Event *event);
private:
    enum { MAX_LINE_LEN = 256, MAX_ESCAPE_LEN = 8, MAX_PROMPT_LEN = 16, MAX_ARG_LEN = 16 };
    enum class State { Input, Debug, Escape };

    class Possibilities
    {
    public:
        Possibilities() : mLen(0), mCount(0) { }
        void append(const char* string)
        {
            unsigned int stringLen = strlen(string);
            strncpy(mString + mLen, string, std::min(sizeof(mString) - 1 - mLen, stringLen));
            mLen += stringLen;
            if (mLen < sizeof(mString) - 1) mString[mLen++] = ' ';
            mString[mLen] = 0;
            ++mCount;
        }
        const char* string() { return mString; }
        unsigned int count() { return mCount; }

    private:
        char mString[256];
        unsigned int mLen;
        unsigned int mCount;
    };
    StmSystem& mSystem;
    std::vector<Command*> mCmd;
    char mLine[MAX_LINE_LEN];
    unsigned int mLineLen;
    char mPrompt[MAX_PROMPT_LEN];
    State mState;
    char mReadChar;
    System::Event mCharReceived;
    CircularBuffer<char*> mHistory;
    int mHistoryIndex;
    char mEscape[MAX_ESCAPE_LEN];
    unsigned int mEscapeLen;

    void printLine();
    Command* findCommand(const char* name, unsigned int len, Possibilities& possible);
    void complete();
    void execute();
    void addToHistory(const char* line);
};

#endif // COMMANDINTERPRETER_H
