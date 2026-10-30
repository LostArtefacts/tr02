#include "game.h"

#define MAX_FLARE_AGE (60 * 30)
#define FLARE_YOUNG 30
#define FLARE_OLD (MAX_FLARE_AGE - 60)

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

int DoFlareLight(PHD_VECTOR *pos, int flare_age)
{
    int x, y, z, random;

    if (flare_age >= MAX_FLARE_AGE)
        return 0;

    random = GetRandomDraw();
    x = pos->x + (random & 0xf);
    y = pos->y;
    z = pos->z;

    if (flare_age < FLARE_YOUNG)
        AddDynamicLight(x, y, z, 12 + ((flare_age - FLARE_YOUNG) * 12) / 60, 11);
    else if (flare_age < FLARE_OLD)
        AddDynamicLight(x, y, z, 12, 11);
    else if (random > 0x2000)
        AddDynamicLight(x, y, z, 12 - (random & 3), 11);
    else
    {
        AddDynamicLight(x, y, z, 12, 5);
        return 0;
    }
    return 1;
}

void set_flare_arm(int frame)
{
    int anim_base;

    anim_base = objects[FLARE].anim_index;
    if (frame < FL_THROW_F)
        lara.left_arm.anim_number = anim_base;
    else if (frame < FL_DRAW_F)
        lara.left_arm.anim_number = anim_base + 1;
    else if (frame < FL_IGNITE_F)
        lara.left_arm.anim_number = anim_base + 2;
    else if (frame < FL_2HOLD_F)
        lara.left_arm.anim_number = anim_base + 3;
    else
        lara.left_arm.anim_number = anim_base + 4;
    lara.left_arm.frame_base = anims[lara.left_arm.anim_number].frame_ptr;
}

void draw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[FLARE].mesh_index + HAND_L];
}

void undraw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void ready_flare(void)
{
    lara.gun_status = LG_ARMLESS;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target = NULL;
}
