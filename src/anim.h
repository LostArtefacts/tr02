#ifndef ANIM_H
#define ANIM_H

#include "types.h"
#include "items.h"
#include "collide.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 room_number;
    sint16 box_number;
} GAME_VECTOR;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 data;
    sint16 flags;
} OBJECT_VECTOR;

typedef struct {
    sint16 *frame_ptr;
    sint16 interpolation;
    sint16 current_anim_state;
    sint32 velocity;
    sint32 acceleration;
    sint16 frame_base;
    sint16 frame_end;
    sint16 jump_anim_num;
    sint16 jump_frame_num;
    sint16 number_changes;
    sint16 change_index;
    sint16 number_commands;
    sint16 command_index;
} ANIM_STRUCT;

typedef struct object_info {
    sint16 nmeshes;
    sint16 mesh_index;
    sint32 bone_index;
    sint16 *frame_base;
    void (*initialise)(sint16 item_number);
    void (*control)(sint16 item_number);
    void (*floor)(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
    void (*ceiling)(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
    void (*draw_routine)(ITEM_INFO *item);
    void (*collision)(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
    sint16 anim_index;
    sint16 hit_points;
    sint16 pivot_length;
    sint16 radius;
    sint16 shadow_size;
    uint16 loaded:1;
    uint16 intelligent:1;
    uint16 save_position:1;
    uint16 save_hitpoints:1;
    uint16 save_flags:1;
    uint16 save_anim:1;
    uint16 semi_transparent:1;
    uint16 water_creature:1;
} OBJECT_INFO;

extern ANIM_STRUCT *anims;
extern OBJECT_INFO objects[];
extern sint16 **meshes;
extern sint32 *bones;

#endif
