#include "game.h"
#include "savegame.h"

extern int SG_Count;
extern char *SG_Point;

void ResetSG(void)
{
    SG_Count = 0;
    SG_Point = savegame.buffer;
}


void S_ExitSystem(char *message);

void WriteSG(void *pointer, int size)
{
    char *data;

    SG_Count += size;
    if (SG_Count >= MAX_SAVEGAME_BUFFER)
        S_ExitSystem("FATAL: Savegame is too big to fit in buffer");

    data = pointer;
    for (; size > 0; size--)
        *(SG_Point++) = *(data++);
}
