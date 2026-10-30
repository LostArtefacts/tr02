#include "game.h"

#define SMASH_WINDOW 72
#define BELL 88

void find_target_point(ITEM_INFO *item, GAME_VECTOR *target)
{
    sint16 *bounds;
    sint32 x, y, z, c, s;

    bounds = GetBestFrame(item);
    x = (sint32)((*(bounds + 0) + *(bounds + 1)) / 2);
    y = (sint32)(*(bounds + 2) + (*(bounds + 3) - *(bounds + 2)) / 3);
    z = (sint32)((*(bounds + 4) + *(bounds + 5)) / 2);
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    target->x = item->pos.x_pos + ((c * x + s * z) >> W2V_SHIFT);
    target->y = item->pos.y_pos + y;
    target->z = item->pos.z_pos + ((c * z - s * x) >> W2V_SHIFT);
    target->room_number = item->room_number;
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

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

void LaraTargetInfo(WEAPON_INFO *winfo)
{
    GAME_VECTOR src, target;
    PHD_ANGLE ang[2];
    ITEM_INFO *item;

    if ((item = lara.target) == NULL)
    {
        lara.left_arm.lock = lara.right_arm.lock = 0;
        lara.target_angles[0] = lara.target_angles[1] = 0;
        return;
    }

    src.x = lara_item->pos.x_pos;
    src.y = lara_item->pos.y_pos - 650;
    src.z = lara_item->pos.z_pos;
    src.room_number = lara_item->room_number;
    find_target_point(item, &target);
    phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);

    ang[0] -= lara_item->pos.y_rot;
    ang[1] -= lara_item->pos.x_rot;

    if (LOS(&src, &target))
    {
        if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] && ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3])
            lara.left_arm.lock = lara.right_arm.lock = 1;
        else
        {
            if (lara.left_arm.lock)
            {
                if (ang[0] < winfo->left_angles[0] || ang[0] > winfo->left_angles[1] || ang[1] < winfo->left_angles[2] || ang[1] > winfo->left_angles[3])
                    lara.left_arm.lock = 0;
            }
            if (lara.right_arm.lock)
            {
                if (ang[0] < winfo->right_angles[0] || ang[0] > winfo->right_angles[1] || ang[1] < winfo->right_angles[2] || ang[1] > winfo->right_angles[3])
                    lara.right_arm.lock = 0;
            }
        }
    }
    else
        lara.left_arm.lock = lara.right_arm.lock = 0;

    lara.target_angles[0] = ang[0];
    lara.target_angles[1] = ang[1];
}

void SmashWindow(sint16 item_number);

void SmashItem(sint16 item_number, int weapon_type)
{
    ITEM_INFO *item;

    item = &items[item_number];
    switch (item->object_number)
    {
    case SMASH_WINDOW:
        SmashWindow(item_number);
        break;

    case BELL:
        if (item->status != ACTIVE)
        {
            item->status = ACTIVE;
            AddActiveItem(item_number);
        }
        break;
    }
}

int WeaponObject(int weapon_type)
{
    switch (weapon_type)
    {
    case LG_PISTOLS:
        return PISTOLS;
    case LG_MAGNUMS:
        return MAGNUM;
    case LG_UZIS:
        return UZI;
    case LG_SHOTGUN:
        return SHOTGUN;
    case LG_GRENADE:
        return GRENADE_GUN;
    case LG_HARPOON:
        return HARPOON_GUN;
    case LG_M16:
        return M16;
    }
}
