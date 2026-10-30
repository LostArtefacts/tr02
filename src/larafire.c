#include "game.h"

int WeaponObject(int weapon_type)
{
    switch (weapon_type)
    {
    case LG_PISTOLS:
        return PISTOLS;
    case LG_MAGNUMS:
        return MAGNUM;
    case LG_UZIS:
        return UZI;
    case LG_SHOTGUN:
        return SHOTGUN;
    case LG_GRENADE:
        return GRENADE_GUN;
    case LG_HARPOON:
        return HARPOON_GUN;
    case LG_M16:
        return M16;
    }
}
