#ifndef CAMERA_H
#define CAMERA_H

#include "anim.h"

#define GAME_FOV 80

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
    sint32 number_frames, bounce;
} CAMERA_INFO;

extern CAMERA_INFO camera;

#endif
