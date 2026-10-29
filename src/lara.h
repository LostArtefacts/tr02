#ifndef LARA_H
#define LARA_H

#include "types.h"
#include "items.h"

#define ONE_DEGREE 182

enum lara_gun_states {
    LG_ARMLESS,
    LG_HANDSBUSY,
    LG_DRAW,
    LG_UNDRAW,
    LG_READY,
    LG_SPECIAL
};

typedef struct lara_info {
    sint16 item_number;
    sint16 gun_status;
    sint16 gun_type;
    sint16 request_gun_type;
    sint16 last_gun_type;
    sint16 calc_fallspeed;
    sint16 water_status;
    sint16 climb_status;
    sint16 pose_count;
    sint16 hit_frame;
    sint16 hit_direction;
    sint16 air;
    sint16 dive_count;
    sint16 death_count;
    sint16 current_active;
    sint16 spaz_effect_count;
    sint16 flare_age;
    sint16 skidoo;
    sint16 weapon_item;
    sint16 back_gun;
    sint16 flare_frame;
    uint16 flare_control_left:1;
    uint16 flare_control_right:1;
    uint16 extra_anim:1;
    uint16 look:1;
} LARA_INFO;

extern LARA_INFO lara;
extern ITEM_INFO *lara_item;

#endif
