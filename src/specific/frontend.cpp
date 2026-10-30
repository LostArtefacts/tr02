#include "global.h"
#include "../inventry.h"
#include "frontend.h"
#include "display.h"
#include "input.h"
#include "output.h"
#include "fmv.h"
#include "time.h"

SG_COL S_COLOUR(int R, int G, int B)
{
    return (SG_COL)SWR_FindNearestPaletteEntry(game_palette, R, G, B, false);
}

void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags)
{
    InsertLine(sx, sy, sx + w, sy + h, phd_znear + (z << 3), (char)col);
}

void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags)
{
#define B_YELLOW 15
#define B_ORANGE 31
    S_DrawScreenLine(sx, sy - 1, z, w + 1, 0, B_YELLOW, NULL, flags);
    S_DrawScreenLine(sx + 1, sy, z, w - 1, 0, B_ORANGE, NULL, flags);
    S_DrawScreenLine(sx + w, sy + 1, z, 0, h - 1, B_YELLOW, NULL, flags);
    S_DrawScreenLine(sx + w + 1, sy, z, 0, h + 1, B_ORANGE, NULL, flags);
    S_DrawScreenLine(sx - 1, sy - 1, z, 0, h + 1, B_YELLOW, NULL, flags);
    S_DrawScreenLine(sx, sy, z, 0, h - 1, B_ORANGE, NULL, flags);
    S_DrawScreenLine(sx, sy + h, z, w - 1, 0, B_YELLOW, NULL, flags);
    S_DrawScreenLine(sx - 1, sy + h + 1, z, w + 1, 0, B_ORANGE, NULL, flags);
}

void S_FinishInventory(void)
{
    if (Inventory_Mode != INV_TITLE_MODE)
        TempVideoRemove();
}

void S_FadeToBlack(void)
{
    int i;
    uint8 *p;

    for (i = 0, p = game_palette; i < 256 * 3; i++, p++)
        *p = 0;
    FadeToPal(10, game_palette);
    FadeWait();
    ScreenClear();
    ScreenDump();
    ScreenClear();
    ScreenDump();
}

int S_PlayFMV(char *fname, int sequence)
{
    return FMV_Play(fname);
}

int S_IntroFMV(char *movie1, char *movie2)
{
    return FMV_PlayIntro(movie1, movie2);
}
