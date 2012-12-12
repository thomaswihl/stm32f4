#ifndef FPUCONTROL_H
#define FPUCONTROL_H

#include "System.h"

class FpuControl
{
public:
    enum class AccessPrivileges { Denied = 0, PrivilegedOnly = 1, Full = 3 };
    FpuControl(System::BaseAddress base);

    void enable(AccessPrivileges privileges);

private:
    struct FPU
    {
        struct __CPACR
        {
            uint32_t __RESERVED0 : 20;
            uint32_t CP : 2;
            uint32_t __RESERVED1 : 8;
        }   CPACR;
        struct __FPCCR
        {
            uint32_t LSPACT : 1;
            uint32_t USER : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t THREAD : 1;
            uint32_t HFRDY : 1;
            uint32_t MMRDY : 1;
            uint32_t BFRDY : 1;
            uint32_t __RESERVED1 : 1;
            uint32_t MONRDY : 1;
            uint32_t __RESERVED2 : 21;
            uint32_t LSPEN : 1;
            uint32_t ASPEN : 1;
        };
        uint32_t FPCAR;
        struct __FPSCR
        {
            uint32_t IOC : 1;
            uint32_t DZC : 1;
            uint32_t OFC : 1;
            uint32_t UFC : 1;
            uint32_t IXC : 1;
            uint32_t __RESERVED0 : 2;
            uint32_t IDC : 1;
            uint32_t __RESERVED1 : 14;
            uint32_t RMODE : 2;
            uint32_t FZ : 1;
            uint32_t DN : 1;
            uint32_t AHP : 1;
            uint32_t __RESERVED2 : 1;
            uint32_t V : 1;
            uint32_t C : 1;
            uint32_t Z : 1;
            uint32_t N : 1;
        }   FPSCR;
        struct __FPDSCR
        {
            uint32_t __RESERVED1 : 22;
            uint32_t RMODE : 2;
            uint32_t FZ : 1;
            uint32_t DN : 1;
            uint32_t AHP : 1;
            uint32_t __RESERVED2 : 5;
        }   FPDSCR;
    };
    volatile FPU* mBase;
};

#endif // FPUCONTROL_H
