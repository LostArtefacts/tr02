#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "anim.h"
#include "items.h"

#define GAME_FOV 80

#define FOLLOW_CENTRE 1
#define NO_CHUNKY 2
#define CHASE_OBJECT 3

typedef enum camera_type {
    CHASE_CAMERA,
    FIXED_CAMERA,
    LOOK_CAMERA,
    COMBAT_CAMERA,
    CINEMATIC_CAMERA,
    HEAVY_CAMERA
} CAMERA_TYPE;

typedef struct {
    GAME_VECTOR pos;
    GAME_VECTOR target;
    CAMERA_TYPE type;
    sint32 shift, flags, fixed_camera;
    sint32 number_frames, bounce, underwater;
    sint32 target_distance, target_square;
    sint16 target_angle, actual_angle;
    sint16 target_elevation;
    sint16 box;
    sint16 number, last, timer, speed;
    ITEM_INFO *item, *last_item;
    OBJECT_VECTOR *fixed;
    int mike_at_lara;
    PHD_VECTOR mike_pos;
} CAMERA_INFO;

extern CAMERA_INFO camera;

#ifdef __cplusplus
}
#endif

#endif
