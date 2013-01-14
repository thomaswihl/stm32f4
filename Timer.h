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

#ifndef TIMER_H
#define TIMER_H

#include "System.h"

#include <stdint.h>

class Timer
{
public:
    Timer(System::BaseAddress base);
private:
    struct CCMR_OUTPUT
    {
        uint8_t CCS : 2;
        uint8_t OCFE : 1;
        uint8_t OCPE : 1;
        uint8_t OCM : 3;
        uint8_t OCCE : 1;
    };
    struct CCMR_INPUT
    {
        uint8_t CCS : 2;
        uint8_t ICPSC : 2;
        uint8_t ICF : 4;
    };

    struct TIMER
    {
        struct __CR1
        {
            uint16_t CEN : 1;
            uint16_t UDIS : 1;
            uint16_t URS : 1;
            uint16_t OPM : 1;
            uint16_t DIR : 1;
            uint16_t CMS : 2;
            uint16_t ARPE : 1;
            uint16_t CKD : 2;
            uint16_t __RESERVED0 : 6;
        }   CR1;
        uint16_t __RESERVED0;
        struct __CR2
        {
            uint16_t CCPC : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t CCUS : 1;
            uint16_t CCDS : 1;
            uint16_t MMS : 3;
            uint16_t TI1S : 1;
            uint16_t OIS1 : 1;
            uint16_t OIS1N : 1;
            uint16_t OIS2 : 1;
            uint16_t OIS2N : 1;
            uint16_t OIS3 : 1;
            uint16_t OIS3N : 1;
            uint16_t OIS4 : 1;
            uint16_t __RESERVED1 : 1;
        }   CR2;
        uint16_t __RESERVED1;
        struct __SMCR
        {
            uint16_t SMS : 3;
            uint16_t __RESERVED0 : 1;
            uint16_t TS : 3;
            uint16_t MSM : 1;
            uint16_t ETF : 4;
            uint16_t ETPS : 2;
            uint16_t ECE : 1;
            uint16_t ETP : 1;
        }   SMCR;
        uint16_t __RESERVED2;
        struct __DIER
        {
            uint16_t UIE : 1;
            uint16_t CC1IE : 1;
            uint16_t CC2IE : 1;
            uint16_t CC3IE : 1;
            uint16_t CC4IE : 1;
            uint16_t COMIE : 1;
            uint16_t TIE : 1;
            uint16_t BIE : 1;
            uint16_t UDE : 1;
            uint16_t CC1DE : 1;
            uint16_t CC2DE : 1;
            uint16_t CC3DE : 1;
            uint16_t CC4DE : 1;
            uint16_t COMDE : 1;
            uint16_t TDE : 1;
            uint16_t __RESERVED0 : 1;
        }   DIER;
        uint16_t __RESERVED3;
        struct __SR
        {
            uint16_t UIF : 1;
            uint16_t CC1IF : 1;
            uint16_t CC2IF : 1;
            uint16_t CC3IF : 1;
            uint16_t CC4IF : 1;
            uint16_t COMIF : 1;
            uint16_t TIF : 1;
            uint16_t BIF : 1;
            uint16_t __RESERVED0 : 1;
            uint16_t CC1OF : 1;
            uint16_t CC2OF : 1;
            uint16_t CC3OF : 1;
            uint16_t CC4OF : 1;
            uint16_t __RESERVED1 : 3;
        }   SR;
        uint16_t __RESERVED4;
        struct __EGR
        {
            uint16_t UG : 1;
            uint16_t CC1G : 1;
            uint16_t CC2G : 1;
            uint16_t CC3G : 1;
            uint16_t CC4G : 1;
            uint16_t COMG : 1;
            uint16_t TG : 1;
            uint16_t BG : 1;
            uint16_t __RESERVED0 : 8;
        }   EGR;
        uint16_t __RESERVED5;
        uint16_t CCMR1;
        uint16_t __RESERVED6;
        uint16_t CCMR2;
        uint16_t __RESERVED7;
        struct __CCER
        {
            uint16_t CC1E : 1;
            uint16_t CC1P : 1;
            uint16_t CC1NE : 1;
            uint16_t CC1NP : 1;
            uint16_t CC2E : 1;
            uint16_t CC2P : 1;
            uint16_t CC2NE : 1;
            uint16_t CC2NP : 1;
            uint16_t CC3E : 1;
            uint16_t CC3P : 1;
            uint16_t CC3NE : 1;
            uint16_t CC3NP : 1;
            uint16_t CC4E : 1;
            uint16_t CC4P : 1;
            uint16_t __RESERVED0 : 2;
        }   CCER;
        uint16_t __RESERVED8;
        uint32_t CNT;
        uint16_t PSC;
        uint16_t __RESERVED9;
        uint32_t ARR;
        struct __RCR
        {
            uint16_t REP : 8;
            uint16_t __RESERVED0 : 8;
        }   RCR;
        uint16_t __RESERVEDA;
        uint32_t CCR1;
        uint32_t CCR2;
        uint32_t CCR3;
        uint32_t CCR4;
        struct __BDTR
        {
            uint16_t DTG : 8;
            uint16_t LOCK : 2;
            uint16_t OSSI : 1;
            uint16_t OSSR : 1;
            uint16_t BKE : 1;
            uint16_t BKP : 1;
            uint16_t AOE : 1;
            uint16_t MOE : 1;
            uint16_t __RESERVED0 : 8;
        }   BDTR;
        uint16_t __RESERVEDB;
        struct __DCR
        {
            uint16_t DBA : 5;
            uint16_t __RESERVED0 : 3;
            uint16_t DBL : 5;
            uint16_t __RESERVED1 : 3;
        };
        uint16_t __RESERVEDC;
        uint16_t DMAR;
        uint16_t __RESERVEDD;
    };
    volatile TIMER* mBase;
};

#endif // TIMER_H
