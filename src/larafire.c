#include "game.h"
#include <stdio.h>
#include "box.h"
#include "sphere.h"
#include "savegame.h"

#define SMASH_WINDOW 72
#define BELL 88

void find_target_point(ITEM_INFO *item, GAME_VECTOR *target);

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

extern sint16 HoldStates[14];

#define FLAREBOX_ITEM 151
#define GUN_ITEM 135

extern WEAPON_INFO weapons[];

int Inv_RequestItem(int itemNum);
void CreateFlare(int thrown);
void undraw_flare_meshes(void);
void draw_pistols(int weapon_type);
void undraw_pistols(int weapon_type);
void draw_shotgun(int weapon_type);
void undraw_shotgun(int weapon_type);
void draw_flare(void);
void undraw_flare(void);
void PistolHandler(int weapon_type);
void RifleHandler(int weapon_type);
int CheckForHoldingState(int state);
void DoFlareInHand(int flare_age);
void set_flare_arm(int frame);
void InitialiseNewWeapon(void);

void LaraGun(void)
{
    AMMO_INFO *ammo;

    if (lara.left_arm.flash_gun > 0)
        lara.left_arm.flash_gun--;
    if (lara.right_arm.flash_gun > 0)
        lara.right_arm.flash_gun--;

    if (lara_item->hit_points <= 0)
        lara.gun_status = LG_ARMLESS;
    else if (lara.gun_status == LG_ARMLESS) {
        if (input & IN_DRAW)
            lara.request_gun_type = lara.last_gun_type;
        else if (input & IN_F) {
            if (lara.gun_type == LG_FLARE)
                lara.gun_status = LG_UNDRAW;
            else if (Inv_RequestItem(FLAREBOX_ITEM))
                lara.request_gun_type = LG_FLARE;
        }

        if (lara.request_gun_type != lara.gun_type || (input & IN_DRAW)) {
            if (lara.request_gun_type == LG_FLARE
                || (lara.skidoo == NO_ITEM
                    && (lara.request_gun_type == LG_HARPOON || lara.water_status == LARA_ABOVEWATER
                        || (lara.water_status == LARA_WADE
                            && lara.water_surface_dist > -weapons[lara.gun_type].gun_height)))) {
                if (lara.gun_type == LG_FLARE) {
                    CreateFlare(0);
                    undraw_flare_meshes();
                    lara.flare_control_left = 0;
                }
                lara.gun_type = lara.request_gun_type;
                InitialiseNewWeapon();
                lara.gun_status = LG_DRAW;
                lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
            } else {
                lara.last_gun_type = lara.request_gun_type;
                if (lara.gun_type == LG_FLARE)
                    lara.request_gun_type = LG_FLARE;
                else
                    lara.gun_type = lara.request_gun_type;
            }
        }
    } else if (lara.gun_status == LG_READY) {
        if ((input & IN_F) && Inv_RequestItem(FLAREBOX_ITEM))
            lara.request_gun_type = LG_FLARE;

        if ((input & IN_DRAW) || lara.request_gun_type != lara.gun_type)
            lara.gun_status = LG_UNDRAW;
        else if (lara.gun_type != LG_HARPOON && lara.water_status != LARA_ABOVEWATER
            && (lara.water_status != LARA_WADE
                || lara.water_surface_dist < -weapons[lara.gun_type].gun_height))
            lara.gun_status = LG_UNDRAW;
    }

    switch (lara.gun_status) {
    case LG_DRAW:
        if (lara.gun_type != LG_FLARE && lara.gun_type != LG_UNARMED)
            lara.last_gun_type = lara.gun_type;
        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_MAGNUMS:
        case LG_UZIS:
            if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
                camera.type = COMBAT_CAMERA;
            draw_pistols(lara.gun_type);
            break;

        case LG_SHOTGUN:
        case LG_M16:
        case LG_GRENADE:
        case LG_HARPOON:
            if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
                camera.type = COMBAT_CAMERA;
            draw_shotgun(lara.gun_type);
            break;

        case LG_FLARE:
            draw_flare();
            break;

        default:
            lara.gun_status = LG_ARMLESS;
            break;
        }
        break;

    case LG_UNDRAW:
        lara.mesh_ptrs[HEAD] = meshes[objects[LARA].mesh_index + HEAD];
        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_MAGNUMS:
        case LG_UZIS:
            undraw_pistols(lara.gun_type);
            break;

        case LG_SHOTGUN:
        case LG_M16:
        case LG_GRENADE:
        case LG_HARPOON:
            undraw_shotgun(lara.gun_type);
            break;

        case LG_FLARE:
            undraw_flare();
            break;

        default:
            break;
        }
        break;

    case LG_SPECIAL:
        draw_flare();
        break;

    case LG_READY:
        if (lara.pistols.ammo && (input & IN_ACTION))
            lara.mesh_ptrs[HEAD] = meshes[objects[UZI].mesh_index + HEAD];
        else
            lara.mesh_ptrs[HEAD] = meshes[objects[LARA].mesh_index + HEAD];

        if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
            camera.type = COMBAT_CAMERA;

        if (input & IN_ACTION) {
            switch (lara.gun_type) {
            case LG_SHOTGUN:
                ammo = &lara.shotgun;
                break;
            case LG_MAGNUMS:
                ammo = &lara.magnums;
                break;
            case LG_UZIS:
                ammo = &lara.uzis;
                break;
            case LG_GRENADE:
                ammo = &lara.grenade;
                break;
            case LG_HARPOON:
                ammo = &lara.harpoon;
                break;
            case LG_M16:
                ammo = &lara.m16;
                break;
            default:
                ammo = &lara.pistols;
                break;
            }

            if (ammo->ammo <= 0) {
                ammo->ammo = 0;
                SoundEffect(48, &lara_item->pos, 0);
                if (Inv_RequestItem(GUN_ITEM))
                    lara.request_gun_type = LG_PISTOLS;
                else
                    lara.request_gun_type = LG_UNARMED;
                break;
            }
        }

        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_MAGNUMS:
        case LG_UZIS:
            PistolHandler(lara.gun_type);
            break;

        case LG_SHOTGUN:
        case LG_M16:
        case LG_GRENADE:
        case LG_HARPOON:
            RifleHandler(lara.gun_type);
            break;
        }
        break;

    case LG_ARMLESS:
        if (lara.gun_type == LG_FLARE) {
            if (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state)) {
                if (!lara.flare_control_left) {
                    lara.left_arm.frame_number = FL_2HOLD_F;
                    lara.flare_control_left = 1;
                } else if (lara.left_arm.frame_number != FL_HOLD_F) {
                    lara.left_arm.frame_number++;
                    if (lara.left_arm.frame_number == FL_END_F)
                        lara.left_arm.frame_number = FL_HOLD_F;
                }
            } else
                lara.flare_control_left = 0;
            goto flare_in_hand;
        }
        break;

    case LG_HANDSBUSY:
        if (lara.gun_type == LG_FLARE) {
            lara.flare_control_left = (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state));
        flare_in_hand:
            DoFlareInHand(lara.flare_age);
            set_flare_arm(lara.left_arm.frame_number);
        }
        break;
    }
}

int CheckForHoldingState(int state)
{
    sint16 *holds = HoldStates;

    if (lara.extra_anim)
        return 0;

    while (*holds >= 0) {
        if (state == *holds)
            return 1;
        holds++;
    }
    return 0;
}

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

#define NEAR_ANGLE (ONE_DEGREE * 15)
#define NUM_SLOTS 5

extern CREATURE_INFO *baddie_slots;

void LaraGetNewTarget(WEAPON_INFO *winfo)
{
    int x, y, z, slot;
    int dist, maxdist, maxdist2, bestdist;
    ITEM_INFO *item, *bestitem;
    PHD_ANGLE ang[2], bestyrot, yrot;
    GAME_VECTOR src, target;
    CREATURE_INFO *creature;

    src.x = lara_item->pos.x_pos;
    src.y = lara_item->pos.y_pos - 650;
    src.z = lara_item->pos.z_pos;
    src.room_number = lara_item->room_number;
    bestitem = NULL;
    bestyrot = 32767;
    bestdist = 0x7fffffff;
    maxdist = winfo->target_dist;
    maxdist2 = maxdist * maxdist;
    creature = baddie_slots;
    for (slot = 0; slot < NUM_SLOTS; slot++, creature++)
    {
        if (creature->item_num == NO_ITEM || creature->item_num == lara.item_number)
            continue;
        item = &items[creature->item_num];
        if (item->hit_points <= 0)
            continue;
        x = item->pos.x_pos - src.x;
        y = item->pos.y_pos - src.y;
        z = item->pos.z_pos - src.z;
        if (ABS(x) > maxdist || ABS(y) > maxdist || ABS(z) > maxdist)
            continue;
        dist = x * x + y * y + z * z;
        if (dist < maxdist2)
        {
            find_target_point(item, &target);
            if (LOS(&src, &target))
            {
                phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
                ang[0] -= lara_item->pos.y_rot + lara.torso_y_rot;
                ang[1] -= lara_item->pos.x_rot + lara.torso_x_rot;
                if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] &&
                    ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3])
                {
                    yrot = ABS(ang[0]);
                    if (yrot < bestyrot + NEAR_ANGLE && dist < bestdist)
                    {
                        bestdist = dist;
                        bestyrot = yrot;
                        bestitem = item;
                    }
                }
            }
        }
    }
    lara.target = bestitem;
    LaraTargetInfo(winfo);
}

void SmashWindow(sint16 item_number);

sint16 *GetBestFrame(ITEM_INFO *);

void find_target_point(ITEM_INFO *item, GAME_VECTOR *target)
{
    sint32 x, z, c, y, s;
    sint16 *bounds;

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

void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm)
{
    PHD_ANGLE curr, speed;
    PHD_ANGLE destx, desty;

    speed = winfo->aim_speed;
    if (arm->lock)
    {
        desty = lara.target_angles[0];
        destx = lara.target_angles[1];
    }
    else
        desty = destx = 0;

    curr = arm->y_rot;
    if (curr >= desty - speed && curr <= desty + speed)
        curr = desty;
    else if (curr < desty)
        curr += speed;
    else
        curr -= speed;
    arm->y_rot = curr;

    curr = arm->x_rot;
    if (curr >= destx - speed && curr <= destx + speed)
        curr = destx;
    else if (curr < destx)
        curr += speed;
    else
        curr -= speed;
    arm->x_rot = curr;
    arm->z_rot = 0;
}

int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target);
void SmashItem(sint16 item_number, int weapon_type);
void HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int damage);
void Richochet(GAME_VECTOR *pos);

extern WEAPON_INFO weapons[];

int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles)
{
    int r, i, nums, bestdist, best;
    AMMO_INFO *ammo;
    WEAPON_INFO *winfo;
    PHD_3DPOS view;
    SPHERE *sptr, slist[33];
    GAME_VECTOR vsrc, vdest;
    sint16 smash_item;

    switch (weapon_type)
    {
    default:
        ammo = &lara.pistols;
        ammo->ammo = 1000;
        break;

    case LG_MAGNUMS:
        ammo = &lara.magnums;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;

    case LG_UZIS:
        ammo = &lara.uzis;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;

    case LG_M16:
        ammo = &lara.m16;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;

    case LG_SHOTGUN:
        ammo = &lara.shotgun;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;
    }
    if (ammo->ammo > 0)
    {
        ammo->ammo--;
        winfo = &weapons[weapon_type];
        view.x_pos = src->pos.x_pos;
        view.y_pos = src->pos.y_pos - winfo->gun_height;
        view.z_pos = src->pos.z_pos;
        r = (int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536;
        view.x_rot = *(angles + 1) + (sint16)r;
        view.y_rot = *(angles + 0) + (sint16)((int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536);
        view.z_rot = 0;
        phd_GenerateW2V(&view);
        nums = GetSpheres(target, slist, 0);
        sptr = &slist[0];
        best = -1;
        bestdist = 0x7fffffff;
        for (i = 0; i < nums; i++, sptr++)
        {
            r = sptr->r;
            if (ABS(sptr->x) < r && ABS(sptr->y) < r && sptr->z > r &&
                (sptr->x * sptr->x) + (sptr->y * sptr->y) <= (r * r))
            {
                if (sptr->z - r < bestdist)
                {
                    bestdist = sptr->z - r;
                    best = i;
                }
            }
        }
        savegame.ammo_used++;
        vsrc.x = view.x_pos;
        vsrc.y = view.y_pos;
        vsrc.z = view.z_pos;
        vsrc.room_number = src->room_number;
        if (best >= 0)
        {
            savegame.ammo_hit++;
            vdest.x = vsrc.x + ((*(phd_mxptr + M20) * bestdist) >> W2V_SHIFT);
            vdest.y = vsrc.y + ((*(phd_mxptr + M21) * bestdist) >> W2V_SHIFT);
            vdest.z = vsrc.z + ((*(phd_mxptr + M22) * bestdist) >> W2V_SHIFT);
            smash_item = ObjectOnLOS(&vsrc, &vdest);
            if (smash_item != NO_ITEM)
                SmashItem(smash_item, weapon_type);
            HitTarget(target, &vdest, winfo->damage);
            return 1;
        }
        else
        {
            vdest.x = vsrc.x + ((*(phd_mxptr + M20) * winfo->target_dist) >> W2V_SHIFT);
            vdest.y = vsrc.y + ((*(phd_mxptr + M21) * winfo->target_dist) >> W2V_SHIFT);
            vdest.z = vsrc.z + ((*(phd_mxptr + M22) * winfo->target_dist) >> W2V_SHIFT);
            best = LOS(&vsrc, &vdest);
            smash_item = ObjectOnLOS(&vsrc, &vdest);
            if (smash_item != NO_ITEM)
                SmashItem(smash_item, weapon_type);
            else if (!best)
                Richochet(&vdest);
            return -1;
        }
    }
    else
    {
        ammo->ammo = 0;
        return 0;
    }
}

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

#ifdef TR2_EXE
sint16 HoldStates[14] = {
    0, 2, 4, 6, 7, 16, 20, 22, 21, 65, 39, 40, 41, -1,
};
#endif
