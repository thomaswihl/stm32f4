#include "FpuControl.h"

FpuControl::FpuControl(System::BaseAddress base) :
    mBase(reinterpret_cast<volatile FPU*>(base))
{
    static_assert(sizeof(FPU) == 0x10, "Struct has wrong size, compiler problem.");
}

void FpuControl::enable(FpuControl::AccessPrivileges privileges)
{
    mBase->CPACR.CP = static_cast<uint32_t>(privileges);
    __asm("DSB");
    __asm("ISB");
}
