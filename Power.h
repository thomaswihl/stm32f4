#ifndef POWER_H
#define POWER_H

#include "System.h"

class Power
{
public:
    Power(System::BaseAddress base);

    void backupDomainWp(bool enable);
private:
    struct PWR
    {
        struct __CR
        {
            uint32_t LPDS : 1;
            uint32_t PDDS : 1;
            uint32_t CWUF : 1;
            uint32_t CSBF : 1;
            uint32_t PVDE : 1;
            uint32_t PLS : 3;
            uint32_t DBP : 1;
            uint32_t FPDS : 1;
            uint32_t __RESERVED0 : 4;
            uint32_t VOS : 1;
            uint32_t __RESERVED1 : 17;
        }   CR;
        struct __CSR
        {
            uint32_t WUF : 1;
            uint32_t SBF : 1;
            uint32_t PVDO : 1;
            uint32_t BRR : 1;
            uint32_t __RESERVED0 : 4;
            uint32_t EWUP : 1;
            uint32_t BRE : 1;
            uint32_t __RESERVED1 : 4;
            uint32_t VOSRDY : 1;
            uint32_t __RESERVED : 17;
        }   CSR;
    };

    volatile PWR* mBase;
};

#endif // POWER_H
