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

#define GUN_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define ROCKET_GUN_ITEM 141
#define SG_AMMO_ITEM 143
#define MAG_AMMO_ITEM 144
#define UZI_AMMO_ITEM 145
#define HARPOON_AMMO_ITEM 146
#define M16_AMMO_ITEM 147
#define ROCKET_AMMO_ITEM 148
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLAREBOX_ITEM 152
#define MAGNUM_AMMO_QTY 40
#define UZI_AMMO_QTY 80
#define SHOTGUN_AMMO_QTY (SHOTGUN_AMMO_CLIP * 2)
#define HARPOON_AMMO_QTY 3
#define M16_AMMO_QTY 40
#define ROCKET_AMMO_QTY 2

int Inv_RequestItem(int itemNum);

void CreateStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];
    start->available = 1;

    if (Inv_RequestItem(GUN_ITEM))
    {
        start->pistol_ammo = 1000;
        start->got_pistols = 1;
    }
    else
    {
        start->pistol_ammo = 1000;
        start->got_pistols = 0;
    }

    if (Inv_RequestItem(MAGNUM_ITEM))
    {
        start->magnum_ammo = lara.magnums.ammo;
        start->got_magnums = 1;
    }
    else
    {
        start->magnum_ammo = Inv_RequestItem(MAG_AMMO_ITEM) * MAGNUM_AMMO_QTY;
        start->got_magnums = 0;
    }

    if (Inv_RequestItem(UZI_ITEM))
    {
        start->uzi_ammo = lara.uzis.ammo;
        start->got_uzis = 1;
    }
    else
    {
        start->uzi_ammo = Inv_RequestItem(UZI_AMMO_ITEM) * UZI_AMMO_QTY;
        start->got_uzis = 0;
    }

    if (Inv_RequestItem(SHOTGUN_ITEM))
    {
        start->shotgun_ammo = lara.shotgun.ammo;
        start->got_shotgun = 1;
    }
    else
    {
        start->shotgun_ammo = Inv_RequestItem(SG_AMMO_ITEM) * SHOTGUN_AMMO_QTY;
        start->got_shotgun = 0;
    }

    if (Inv_RequestItem(HARPOON_ITEM))
    {
        start->harpoon_ammo = lara.harpoon.ammo;
        start->got_harpoon = 1;
    }
    else
    {
        start->harpoon_ammo = Inv_RequestItem(HARPOON_AMMO_ITEM) * HARPOON_AMMO_QTY;
        start->got_harpoon = 0;
    }

    if (Inv_RequestItem(M16_ITEM))
    {
        start->m16_ammo = lara.m16.ammo;
        start->got_m16 = 1;
    }
    else
    {
        start->m16_ammo = Inv_RequestItem(M16_AMMO_ITEM) * M16_AMMO_QTY;
        start->got_m16 = 0;
    }

    if (Inv_RequestItem(ROCKET_GUN_ITEM))
    {
        start->rocket_ammo = lara.grenade.ammo;
        start->got_rocket = 1;
    }
    else
    {
        start->rocket_ammo = Inv_RequestItem(ROCKET_AMMO_ITEM) * ROCKET_AMMO_QTY;
        start->got_rocket = 0;
    }

    start->num_flares = Inv_RequestItem(FLAREBOX_ITEM);
    start->num_medis = Inv_RequestItem(MEDI_ITEM);
    start->num_big_medis = Inv_RequestItem(BIGMEDI_ITEM);

    if (lara.gun_type == LG_FLARE)
        start->gun_type = (char)lara.last_gun_type;
    else
        start->gun_type = (char)lara.gun_type;
    start->gun_status = LG_ARMLESS;
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
