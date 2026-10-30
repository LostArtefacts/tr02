#include "game.h"
#include "savegame.h"

extern int SG_Count;
extern char *SG_Point;

void ResetSG(void)
{
    SG_Count = 0;
    SG_Point = savegame.buffer;
}
