#include "sdio.h"

Sdio::Sdio(System::BaseAddress base, ClockControl *clockControl)
{
    static_assert(sizeof(SDIO) == 0x84, "Struct has wrong size, compiler problem.");
}
