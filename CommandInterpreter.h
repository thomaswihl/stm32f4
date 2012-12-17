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

#include <vector>
#include <memory>

class CommandInterpreter
{
public:
    struct Command
    {
    public:
        Command() { }

        const char* startsWith(const char* string, unsigned int len) const;

        virtual bool execute(CommandInterpreter& interpreter, int argc, const char* argv[]) = 0;
        virtual unsigned int aliases(const char**& alias) const = 0;
        virtual const char* helpText() const = 0;
    };

    typedef std::vector<std::shared_ptr<Command>>::iterator iterator;
    typedef std::vector<std::shared_ptr<Command>>::const_iterator const_iterator;
    iterator begin() { return mCmd.begin(); }
    iterator end() { return mCmd.end(); }

    CommandInterpreter(StmSystem& system);
    void feed();
    void add(std::shared_ptr<Command> cmd);
    void start();
private:
    enum { MAX_LINE_LEN = 256, MAX_PROMPT_LEN = 16 };

    class Possibilities
    {
    public:
        Possibilities() : mLen(0) { }
        void append(const char* string)
        {
            unsigned int stringLen = strlen(string);
            strncpy(mString + mLen, string, std::min(sizeof(mString) - mLen, stringLen));
            mLen += stringLen;
            if (mLen < sizeof(mString)) mString[mLen++] = ' ';
        }
        const char* string() { return mString; }

    private:
        char mString[256];
        unsigned int mLen;
    };
    StmSystem& mSystem;
    std::vector<std::shared_ptr<Command>> mCmd;
    char mLine[MAX_LINE_LEN];
    unsigned int mLineLen;
    char mPrompt[MAX_PROMPT_LEN];

    void printLine();
    void complete();
    void execute();
};

#endif // COMMANDINTERPRETER_H
