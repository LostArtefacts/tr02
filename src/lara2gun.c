#include "game.h"

#define DRAW1_F 5
#define DRAW2_F 13
#define RECOIL_F 24

void set_pistol_arm(LARA_ARM *arm, int frame)
{
    int anim_base;

    anim_base = objects[PISTOLS].anim_index;
    if (frame < DRAW1_F)
        arm->anim_number = anim_base;
    else if (frame < DRAW2_F)
        arm->anim_number = anim_base + 1;
    else if (frame < RECOIL_F)
        arm->anim_number = anim_base + 2;
    else
        arm->anim_number = anim_base + 3;
    arm->frame_base = anims[arm->anim_number].frame_ptr;
    arm->frame_number = frame;
}

void ready_pistols(int weapon_type);
void draw_pistol_meshes(int weapon_type);

void draw_pistols(int weapon_type)
{
    sint16 ani;

    ani = lara.left_arm.frame_number;
    ani++;
    if (ani < DRAW1_F || ani > RECOIL_F - 1)
        ani = DRAW1_F;
    else if (ani == DRAW2_F)
    {
        draw_pistol_meshes(weapon_type);
        SoundEffect(6, &lara_item->pos, 0);
    }
    else if (ani == RECOIL_F - 1)
    {
        ready_pistols(weapon_type);
        ani = 0;
    }
    set_pistol_arm(&lara.right_arm, ani);
    set_pistol_arm(&lara.left_arm, ani);
}

void undraw_pistol_mesh_left(int weapon_type);
void undraw_pistol_mesh_right(int weapon_type);

void undraw_pistols(int weapon_type)
{
    sint16 anil, anir;

    anil = lara.left_arm.frame_number;
    if (anil >= RECOIL_F)
        anil = DRAW1_F - 1;
    else if (anil > 0 && anil < DRAW1_F)
    {
        lara.left_arm.x_rot -= lara.left_arm.x_rot / anil;
        lara.left_arm.y_rot -= lara.left_arm.y_rot / anil;
        anil--;
    }
    else if (anil == 0)
    {
        lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
        anil = RECOIL_F - 1;
    }
    else if (anil > DRAW1_F && anil < RECOIL_F)
    {
        anil--;
        if (anil == DRAW2_F - 1)
        {
            undraw_pistol_mesh_left(weapon_type);
            SoundEffect(7, &lara_item->pos, 0);
        }
    }
    set_pistol_arm(&lara.left_arm, anil);

    anir = lara.right_arm.frame_number;
    if (anir >= RECOIL_F)
        anir = DRAW1_F - 1;
    else if (anir > 0 && anir < DRAW1_F)
    {
        lara.right_arm.x_rot -= lara.right_arm.x_rot / anir;
        lara.right_arm.y_rot -= lara.right_arm.y_rot / anir;
        anir--;
    }
    else if (anir == 0)
    {
        lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
        anir = RECOIL_F - 1;
    }
    else if (anir > DRAW1_F && anir < RECOIL_F)
    {
        anir--;
        if (anir == DRAW2_F - 1)
        {
            undraw_pistol_mesh_right(weapon_type);
            SoundEffect(7, &lara_item->pos, 0);
        }
    }
    set_pistol_arm(&lara.right_arm, anir);

    if (anil == DRAW1_F && anir == DRAW1_F)
    {
        lara.gun_status = LG_ARMLESS;
        lara.right_arm.frame_number = lara.left_arm.frame_number = 0;
        lara.target = NULL;
        lara.left_arm.lock = lara.right_arm.lock = 0;
    }

    if (!(input & IN_LOOK))
    {
        lara.torso_y_rot = lara.head_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
        lara.torso_x_rot = lara.head_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;
    }
}

void ready_pistols(int weapon_type)
{
    lara.gun_status = LG_READY;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.target = NULL;
    lara.left_arm.lock = lara.right_arm.lock = 0;

    lara.left_arm.frame_base = lara.right_arm.frame_base = objects[PISTOLS].frame_base;
}

void draw_pistol_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_L] = meshes[objects[obj].mesh_index + HAND_L];
    lara.mesh_ptrs[HAND_R] = meshes[objects[obj].mesh_index + HAND_R];
    lara.mesh_ptrs[THIGH_L] = meshes[objects[LARA].mesh_index + THIGH_L];
    lara.mesh_ptrs[THIGH_R] = meshes[objects[LARA].mesh_index + THIGH_R];
}

void undraw_pistol_mesh_left(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[THIGH_L] = meshes[objects[obj].mesh_index + THIGH_L];
    lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void undraw_pistol_mesh_right(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[THIGH_R] = meshes[objects[obj].mesh_index + THIGH_R];
    lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
}
