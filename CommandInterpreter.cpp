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
    { "read[bhw]", "address:u length:u:o", "Read memory of LENGTH [bytes|halfwords|words] from ADDRESS .", read },
    { "write[bhw]", "address:u value:u length:u:o", "Read memory of LENGTH [bytes|halfwords|words] from ADDRESS .", write },
};

CommandInterpreter::CommandInterpreter()
{
}

