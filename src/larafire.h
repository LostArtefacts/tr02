#ifndef LARAFIRE_H
#define LARAFIRE_H

#include "3dglodef.h"

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

#endif
