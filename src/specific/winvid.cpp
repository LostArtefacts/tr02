#include "global.h"

extern "C" bool IsMinWindowSizeSet;

extern "C" void WinVidClearMinWindowSize(void)
{
    IsMinWindowSizeSet = false;
}
