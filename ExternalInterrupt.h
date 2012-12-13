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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "InterruptController.h"

#include <cstdint>

class ExternalInterrupt
{
public:
    ExternalInterrupt(unsigned int base, std::size_t vectorSize);
    ~ExternalInterrupt();

    void set(InterruptController::Index index, InterruptController::Handler *handler);
    void handle(InterruptController::Index index);
private:
    struct EXTI
    {
        uint32_t IMR : 23;
        uint32_t : 0;
        uint32_t EMR : 23;
        uint32_t : 0;
        uint32_t RTSR : 23;
        uint32_t : 0;
        uint32_t FTSR : 23;
        uint32_t : 0;
        uint32_t SWIER : 23;
        uint32_t : 0;
        uint32_t PR : 23;
        uint32_t : 0;
    };

    volatile EXTI* mBase;
    InterruptController::Handler** mHandler;

};

#endif // INTERRUPT_H
