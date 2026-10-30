#include "game.h"

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
