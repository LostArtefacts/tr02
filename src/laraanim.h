#ifndef LARAANIM_H
#define LARAANIM_H

#include "anim.h"

enum lara_meshes {
    HIPS,
    THIGH_L, CALF_L, FOOT_L,
    THIGH_R, CALF_R, FOOT_R,
    TORSO,
    UARM_R, LARM_R, HAND_R,
    UARM_L, LARM_L, HAND_L,
    HEAD
};

enum lara_states {
    AS_WALK = 0,
    AS_RUN,
    AS_STOP,
    AS_FORWARDJUMP,
    AS_POSE,
    AS_FASTBACK,
    AS_TURN_R,
    AS_TURN_L,
    AS_DEATH,
    AS_FASTFALL
};

#define LARA_FASTFALL_SPEED (FASTFALL_SPEED + 3)

#define GF(a, b) (anims[a].frame_base + b)

#define STOP_A 11
#define STOP_F GF(STOP_A, 0)

#endif
