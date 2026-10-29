#ifndef COLLIDE_H
#define COLLIDE_H

#include "3dglodef.h"

typedef struct {
    sint32 mid_floor;
    sint32 mid_ceiling;
    sint32 mid_type;
    sint32 front_floor;
    sint32 front_ceiling;
    sint32 front_type;
    sint32 left_floor;
    sint32 left_ceiling;
    sint32 left_type;
    sint32 right_floor;
    sint32 right_ceiling;
    sint32 right_type;
    sint32 radius;
    sint32 bad_pos, bad_neg;
    sint32 bad_ceiling;
    PHD_VECTOR shift;
    PHD_VECTOR old;
    sint16 old_anim_state;
    sint16 old_anim_number;
    sint16 old_frame_number;
    sint16 facing;
    sint16 quadrant;
    sint16 coll_type;
    sint16 *trigger;
    signed char tilt_x, tilt_z;
    char hit_by_baddie;
    char hit_static;
    uint16 slopes_are_walls:1;
    uint16 slopes_are_pits:1;
    uint16 lava_is_pit:1;
    uint16 enable_baddie_push:1;
    uint16 enable_spaz:1;
} COLL_INFO;

sint32 FindGridShift(sint32 src, sint32 dst);
void GetNearByRooms(sint32 x, sint32 y, sint32 z, sint32 r, sint32 h, sint16 room_number);
void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number);
int Move3DPosTo3DPos(PHD_3DPOS *srcpos, PHD_3DPOS *destpos, int velocity, PHD_ANGLE angadd);

#endif
