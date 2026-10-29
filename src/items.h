#ifndef ITEMS_H
#define ITEMS_H

#include "3dglodef.h"

typedef struct item_info {
    sint32 floor;
    uint32 touch_bits;
    uint32 mesh_bits;
    sint16 object_number;
    sint16 current_anim_state;
    sint16 goal_anim_state;
    sint16 required_anim_state;
    sint16 anim_number;
    sint16 frame_number;
    sint16 room_number;
    sint16 next_item;
    sint16 next_active;
    sint16 speed;
    sint16 fallspeed;
    sint16 hit_points;
    uint16 box_number;
    sint16 timer;
    sint16 flags;
    sint16 shade;
    sint16 shadeB;
    sint16 carried_item;
    void *data;
    PHD_3DPOS pos;
} ITEM_INFO;

#endif
