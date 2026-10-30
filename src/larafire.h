#ifndef LARAFIRE_H
#define LARAFIRE_H

#include "3dglodef.h"

enum lara_gun_types {
    LG_UNARMED,
    LG_PISTOLS,
    LG_MAGNUMS,
    LG_UZIS,
    LG_SHOTGUN,
    LG_M16,
    LG_GRENADE,
    LG_HARPOON,
    LG_FLARE,
    LG_SKIDOO,
    NUM_WEAPONS
};

typedef struct lara_arm {
    sint16 *frame_base;
    sint16 frame_number;
    sint16 anim_number;
    sint16 lock;
    PHD_ANGLE y_rot, x_rot, z_rot;
    sint16 flash_gun;
} LARA_ARM;

typedef struct ammo_info {
    int ammo;
} AMMO_INFO;

int WeaponObject(int weapon_type);

#endif
