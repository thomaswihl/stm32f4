#ifndef SYSCFG_H
#define SYSCFG_H

#include "stdint.h"

class SysCfg
{
public:
    SysCfg();

private:
    struct SYSCFG
    {
        struct __MEMRMP
        {
            uint32_t MEM_MODE : 2;
            uint32_t __RESERVED0 : 30;
        }   MEMRMP;
        struct __PMC
        {
            uint32_t __RESERVED0 : 23;
            uint32_t MII_RMII_SEL : 1;
            uint32_t __RESERVED1 : 8;
        }   PMC;
        uint32_t EXTICR[4];
        uint32_t __RESERVED0[2];
        struct __CMPCR
        {
            uint32_t CMP_PD : 1;
            uint32_t __RESERVED0 : 7;
            uint32_t READY : 1;
            uint32_t __RESERVED1 : 23;
        }   CMPCR;
    };
};

#endif // SYSCFG_H
