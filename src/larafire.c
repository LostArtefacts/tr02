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

void draw_pistol_meshes(int weapon_type);
void draw_shotgun_meshes(int weapon_type);
int WeaponObject(int weapon_type);

void InitialiseNewWeapon(void)
{
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.target = NULL;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;

    switch (lara.gun_type)
    {
    case LG_PISTOLS:
    case LG_MAGNUMS:
    case LG_UZIS:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[PISTOLS].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_pistol_meshes(lara.gun_type);
        break;

    case LG_SHOTGUN:
    case LG_M16:
    case LG_GRENADE:
    case LG_HARPOON:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(lara.gun_type)].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_shotgun_meshes(lara.gun_type);
        break;

    case LG_FLARE:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[FLARE].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_flare_meshes();
        break;

    default:
        lara.left_arm.frame_base = lara.right_arm.frame_base = anims[lara_item->anim_number].frame_ptr;
        break;
    }
}

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

#include "box.h"
#include "savegame.h"

#define MONK1 53
#define MONK2 54
#define MONKS_GET_ANGRY 10

extern int monks_attack_lara;

void HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int damage)
{
    CREATURE_INFO *monk;

    if (item->hit_points > 0 && item->hit_points <= damage)
        savegame.kills++;
    item->hit_points -= damage;
    item->hit_status = 1;

    if (hitpos)
        DoBloodSplat(hitpos->x, hitpos->y, hitpos->z, item->speed, item->pos.y_rot, item->room_number);

    if (!monks_attack_lara && (item->object_number == MONK1 || item->object_number == MONK2))
    {
        monk = (CREATURE_INFO *)item->data;
        monk->flags += damage;
        if ((monk->flags & 0x0fff) > MONKS_GET_ANGRY || monk->mood == BORED_MOOD)
            monks_attack_lara = 1;
    }
}

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
