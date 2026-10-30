#include "global.h"
#include "output.h"
#include "../inventry.h"

#define ANIMATED_TEXTURE 10

void S_ClearScreen(void)
{
    ScreenClear();
}

void S_InsertBackPolygon(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int colour)
{
    min_x += phd_winxmin;
    max_x += phd_winxmin;
    min_y += phd_winymin;
    max_y += phd_winymin;
    InsertFlatRect(min_x, min_y, max_x, max_y, phd_zfar + 1, inv_colours[C_BLACK]);
}

void ScreenDump(void)
{
    DX_UpdateFrame(true, 0);
}

void ScreenPartialDump(void)
{
    DX_UpdateFrame(true, &GrcDumpWindow);
}
