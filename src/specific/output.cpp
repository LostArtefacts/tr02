#include "global.h"
#include "output.h"

void S_ClearScreen(void)
{
    ScreenClear();
}

void ScreenDump(void)
{
    DX_UpdateFrame(true, 0);
}
