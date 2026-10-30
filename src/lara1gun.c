#include "game.h"

void draw_shotgun_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_R] = meshes[objects[obj].mesh_index + HAND_R];
    lara.back_gun = 0;
}

void undraw_shotgun_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
    lara.back_gun = obj;
}

void ready_shotgun(int weapon_type)
{
    lara.gun_status = LG_READY;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target = NULL;
    lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
}

typedef struct {
    PHD_ANGLE lock_angles[4];
    PHD_ANGLE left_angles[4];
    PHD_ANGLE right_angles[4];
    PHD_ANGLE aim_speed;
    PHD_ANGLE shot_accuracy;
    int gun_height;
    int damage;
    int target_dist;
    sint16 recoil_frame;
    sint16 flash_time;
    sint16 sample_num;
} WEAPON_INFO;

extern WEAPON_INFO weapons[];

int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles);

void FireM16(int running)
{
    PHD_ANGLE angles[2];

    angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
    angles[1] = lara.left_arm.x_rot;

    if (running)
    {
        weapons[M16].shot_accuracy = ONE_DEGREE * 12;
        weapons[M16].damage = 1;
    }
    else
    {
        weapons[M16].shot_accuracy = ONE_DEGREE * 4;
        weapons[M16].damage = 3;
    }

    if (FireWeapon(LG_M16, lara.target, lara_item, angles))
        lara.right_arm.flash_gun = weapons[LG_M16].flash_time;
}

enum weapon_anim {
    W_AIM,
    W_DRAW,
    W_RECOIL,
    W_UNDRAW,
    W_UNAIM,
    W_RELOAD,
    W_UAIM,
    W_UUNAIM,
    W_URECOIL,
    W_SURF_UNDRAW
};

#define ROCKET_DRAW_ANIM 0
#define HARPOON_DRAW_ANIM 1
#define NO_ROOM 0xff

sint16 CreateItem(void);

void draw_shotgun(int weapon_type)
{
    ITEM_INFO *item;

    if (lara.weapon_item == NO_ITEM)
    {
        lara.weapon_item = CreateItem();
        item = &items[lara.weapon_item];
        item->object_number = WeaponObject(weapon_type);
        if (weapon_type == LG_GRENADE)
            item->anim_number = objects[GRENADE_GUN].anim_index + ROCKET_DRAW_ANIM;
        else
            item->anim_number = objects[item->object_number].anim_index + HARPOON_DRAW_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = W_DRAW;
        item->status = ACTIVE;
        item->room_number = NO_ROOM;
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[item->object_number].frame_base;
    }
    else
        item = &items[lara.weapon_item];

    AnimateItem(item);

    if (item->current_anim_state == W_AIM || item->current_anim_state == W_UAIM)
        ready_shotgun(weapon_type);
    else if (item->frame_number - anims[item->anim_number].frame_base == 10)
        draw_shotgun_meshes(weapon_type);
    else if (lara.water_status == LARA_UNDERWATER)
        item->goal_anim_state = W_UAIM;

    lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
    lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
    lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

