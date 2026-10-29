#include "game.h"

#define HAIR_SEGMENTS 6

static int first_hair;
static PHD_3DPOS hair[HAIR_SEGMENTS + 1];
static PHD_VECTOR hvel[HAIR_SEGMENTS + 1];

void InitialiseHair(void)
{
    int i;
    sint32 *bone;

    first_hair = 1;
    bone = bones + objects[HAIR].bone_index;
    hair[0].y_rot = 0;
    hair[0].x_rot = -0x4000;
    for (i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
    {
        hair[i].x_pos = *(bone + 1);
        hair[i].y_pos = *(bone + 2);
        hair[i].z_pos = *(bone + 3);
        hair[i].x_rot = -0x4000;
        hair[i].y_rot = hair[i].z_rot = 0;
        hvel[i].x = hvel[i].y = hvel[i].z = 0;
    }
}
