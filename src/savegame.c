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

#define SHOTGUN_AMMO_CLIP 6

void ModifyStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];

    start->got_pistols = 1;
    start->gun_type = LG_PISTOLS;
    start->pistol_ammo = 1000;
    switch (level_number)
    {
    case LV_GYM:
        start->available = 1;
        start->pistol_ammo = start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->harpoon_ammo = start->rocket_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_harpoon = start->got_rocket = start->got_m16 = start->got_harpoon = start->got_rocket = 0;
        start->num_medis = start->num_big_medis = start->num_flares = 0;
        start->gun_type = LG_UNARMED;
        start->gun_status = LG_ARMLESS;
        break;

    case LV_FIRSTLEVEL:
        start->available = 1;
        start->got_shotgun = 1;
        start->shotgun_ammo = 2 * SHOTGUN_AMMO_CLIP;
        start->num_flares = 2;
        start->num_medis = 1;
        start->num_big_medis = 1;
        start->got_magnums = start->got_uzis = start->got_m16 = start->got_harpoon = start->got_rocket = 0;
        start->magnum_ammo = start->uzi_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->gun_status = LG_ARMLESS;
        break;
    }
    if (savegame.bonus_flag && level_number != LV_GYM)
    {
        start->available = 1;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_rocket = start->got_m16 = start->got_harpoon = 1;
        start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->rocket_ammo = start->m16_ammo = start->harpoon_ammo = 10001;
        start->gun_type = LG_GRENADE;
        start->num_flares = 255;
    }
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
