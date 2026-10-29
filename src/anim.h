#ifndef ANIM_H
#define ANIM_H

#include "types.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 room_number;
    sint16 box_number;
} GAME_VECTOR;

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

extern ANIM_STRUCT *anims;

#endif
