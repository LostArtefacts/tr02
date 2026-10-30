#include "game.h"
#include "savegame.h"

#define GUN_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define GRENADE_ITEM 141
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLAREBOX_ITEM 151

int Inv_AddItem(int itemNum);

#define GUN_OPTION 157
#define SHOTGUN_OPTION 158
#define MAGNUM_OPTION 159
#define UZI_OPTION 160
#define HARPOON_OPTION 161
#define M16_OPTION 162
#define GRENADE_OPTION 163
#define MEDI_OPTION 171
#define BIGMEDI_OPTION 172
#define FLAREBOX_OPTION 173
#define NUMBER_OBJECTS 265
#define LARA_HITPOINTS 1000

int Inv_RemoveItem(int itemNum);

void UseItem(sint16 object_num)
{
    if (object_num < 0 || object_num >= NUMBER_OBJECTS)
        return;
    switch (object_num)
    {
    case GUN_ITEM:
    case GUN_OPTION:
        lara.request_gun_type = LG_PISTOLS;
        break;

    case MAGNUM_ITEM:
    case MAGNUM_OPTION:
        lara.request_gun_type = LG_MAGNUMS;
        break;

    case UZI_ITEM:
    case UZI_OPTION:
        lara.request_gun_type = LG_UZIS;
        break;

    case SHOTGUN_ITEM:
    case SHOTGUN_OPTION:
        lara.request_gun_type = LG_SHOTGUN;
        break;

    case HARPOON_ITEM:
    case HARPOON_OPTION:
        lara.request_gun_type = LG_HARPOON;
        break;

    case M16_ITEM:
    case M16_OPTION:
        lara.request_gun_type = LG_M16;
        break;

    case GRENADE_ITEM:
    case GRENADE_OPTION:
        lara.request_gun_type = LG_GRENADE;
        break;

    case FLAREBOX_ITEM:
    case FLAREBOX_OPTION:
        lara.request_gun_type = LG_FLARE;
        break;

    case MEDI_ITEM:
    case MEDI_OPTION:
        if (lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS)
        {
            lara_item->hit_points += LARA_HITPOINTS / 2;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(MEDI_ITEM);
            SoundEffect(116, NULL, SFX_ALWAYS);
            savegame.health_used++;
        }
        break;

    case BIGMEDI_ITEM:
    case BIGMEDI_OPTION:
        if (lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS)
        {
            lara_item->hit_points += LARA_HITPOINTS;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(BIGMEDI_ITEM);
            SoundEffect(116, NULL, SFX_ALWAYS);
            savegame.health_used += 2;
        }
        break;

    default:
        break;
    }
}

void LaraCheatGetStuff(void)
{
    int i;

    Inv_AddItem(HARPOON_ITEM);
    Inv_AddItem(M16_ITEM);
    Inv_AddItem(GRENADE_ITEM);
    Inv_AddItem(SHOTGUN_ITEM);
    Inv_AddItem(UZI_ITEM);
    Inv_AddItem(MAGNUM_ITEM);
    Inv_AddItem(GUN_ITEM);
    for (i = 0; i < 10; i++)
    {
        Inv_AddItem(FLAREBOX_ITEM);
        Inv_AddItem(MEDI_ITEM);
        Inv_AddItem(BIGMEDI_ITEM);
    }

    lara.magnums.ammo = 1000;
    lara.uzis.ammo = 2000;
    lara.shotgun.ammo = 300;
    lara.harpoon.ammo = 300;
    lara.m16.ammo = 300;
    lara.grenade.ammo = 300;
}

void ControlLaraExtra(sint16 item_num)
{
    AnimateItem(&items[item_num]);
}

void InitialiseLaraLoad(sint16 item_num)
{
    lara.item_number = item_num;
    lara_item = &items[item_num];
}
