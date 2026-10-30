#include "game.h"
#include "savegame.h"

extern int SG_Count;
extern char *SG_Point;

void ModifyStartInfo(int level_number);

void InitialiseStartInfo(void)
{
    int i;

    if (savegame.bonus_flag)
        return;

    for (i = 0; i < MAXIMUM_LEVELS; i++)
    {
        ModifyStartInfo(i);
        savegame.start[i].available = 0;
        savegame.start[i].timer = 0;
        savegame.start[i].ammo_used = 0;
        savegame.start[i].ammo_hit = 0;
        savegame.start[i].distance_travelled = 0;
        savegame.start[i].kills = 0;
        savegame.start[i].secrets_found = 0;
        savegame.start[i].health_used = 0;
    }
    savegame.start[0].available = 1;
    savegame.start[1].available = 1;
    savegame.bonus_flag = 0;
}

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


void ReadSG(void *pointer, int size)
{
    char *data;

    SG_Count += size;
    data = pointer;
    for (; size > 0; size--)
        *(data++) = *(SG_Point++);
}
