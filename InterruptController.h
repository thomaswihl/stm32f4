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

#ifndef INTERRUPTCONTROLLER_H
#define INTERRUPTCONTROLLER_H

#include <cstdint>

class InterruptController
{
public:
    typedef std::uint8_t Index;
    enum class Priority { Highest, Prio1, Prio2, High, Prio4, MediumHigh, Prio6, Medium, Prio8, MediumLow, Prio10, Low, Prio12, Prio13, Prio14, Lowest };

    InterruptController(unsigned int base, std::size_t vectorSize);
    ~InterruptController();

    void handle(Index index);
    void setPriotity(Index index, Priority priority);
    void trigger(Index index);

    class Callback
    {
    public:
        Callback() { }
        virtual ~Callback() { }
        virtual void interruptCallback(Index index) = 0;
    };

    class Line
    {
    public:
        Line(InterruptController& interruptController, Index index);
        ~Line();
        void setCallback(Callback *handler);
        void enable();
        void disable();
        Index index() const { return mIndex; }
    private:
        InterruptController& mInterruptController;
        Index mIndex;
    };

private:
    struct NVIC
    {
        uint32_t ISER[32];  //0x0000
        uint32_t ICER[32];  //0x0080
        uint32_t ISPR[32];  //0x0100
        uint32_t ICPR[32];  //0x0180
        uint32_t IABR[32];  //0x0200
        uint32_t __RESERVED0[32];  //0x0280
        uint8_t IPR[128];  //0x0300
        uint32_t __RESERVED1[32];  //0x0380
        uint32_t __RESERVED2[640];  //0x0400
        uint32_t STIR;  //0x0e00
    };
    volatile NVIC* mBase;
    InterruptController::Callback** mHandler;
};

#endif // INTERRUPTCONTROLLER_H
