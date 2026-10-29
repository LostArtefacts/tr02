#include "../inventry.h"
#include "display.h"
#include "fmv.h"
#include "frontend.h"

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
