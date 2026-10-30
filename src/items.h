#ifndef ITEMS_H
#define ITEMS_H

#include "3dglodef.h"

#define NOT_ACTIVE 0
#define ACTIVE 1
#define DEACTIVATED 2
#define INVISIBLE 3
#define ONESHOT 0x100

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
    uint16 active:1;
    uint16 status:2;
    uint16 gravity_status:1;
    uint16 hit_status:1;
    uint16 collidable:1;
    uint16 looked_at:1;
    uint16 dynamic_light:1;
    uint16 clear_body:1;
} ITEM_INFO;

extern ITEM_INFO *items;

#define NO_ITEM -1
#define NUM_EFFECTS 100

typedef struct fx_info {
    PHD_3DPOS pos;
    sint16 room_number;
    sint16 object_number;
    sint16 next_fx;
    sint16 next_active;
    sint16 speed;
    sint16 fallspeed;
    sint16 frame_number;
    sint16 counter;
    sint16 shade;
} FX_INFO;

extern FX_INFO *effects;
extern sint16 next_fx_active;
extern sint16 next_fx_free;

void RemoveActiveItem(sint16 item_num);
void AddActiveItem(sint16 item_num);
sint16 CreateEffect(sint16 room_num);
void KillItem(sint16 item_number);

#endif
