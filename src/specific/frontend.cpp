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

void S_FinishInventory(void)
{
    if (Inventory_Mode != INV_TITLE_MODE)
        TempVideoRemove();
}

int S_PlayFMV(char *fname, int sequence)
{
    return FMV_Play(fname);
}

int S_IntroFMV(char *movie1, char *movie2)
{
    return FMV_PlayIntro(movie1, movie2);
}
