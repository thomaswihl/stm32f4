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

class CommandInterpreter
{
public:
    struct Command
    {
    public:
        const char* mName;
        const char* mParam;
        const char* mHelp;
        bool (*execute)(CommandInterpreter&, const char*);

    private:
    };

    CommandInterpreter();
private:
    static const Command mCmd[];
};

#endif // COMMANDINTERPRETER_H
