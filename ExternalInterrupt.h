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

class ExternalInterrupt : public InterruptController::Callback
{
public:
    enum class Trigger { Rising, Falling, RisingAndFalling };
    ExternalInterrupt(unsigned int base, unsigned int vectorSize);
    ~ExternalInterrupt();

    class Line
    {
    public:
        Line(ExternalInterrupt& interruptController, InterruptController::Index index);
        ~Line();
        void setCallback(InterruptController::Callback *handler);
        void enable(Trigger trigger);
        void disable();
        InterruptController::Index index();
    private:
        ExternalInterrupt& mInterruptController;
        InterruptController::Index mIndex;
    };

protected:
    virtual void interruptCallback(InterruptController::Index index);

private:
    struct EXTI
    {
        uint32_t IMR;
        uint32_t EMR;
        uint32_t RTSR;
        uint32_t FTSR;
        uint32_t SWIER;
        uint32_t PR;
    };

    volatile EXTI* mBase;
    unsigned int mVectorSize;
    InterruptController::Callback** mCallback;

};

#endif // INTERRUPT_H
