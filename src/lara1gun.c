#include "game.h"
#include "savegame.h"

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
void LaraTargetInfo(WEAPON_INFO *winfo);
void LaraGetNewTarget(WEAPON_INFO *winfo);
void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
void AnimateShotgun(int weapon_type);
void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

void RifleHandler(int weapon_type)
{
    WEAPON_INFO *winfo;

    winfo = &weapons[weapon_type];
    if (input & IN_ACTION)
        LaraTargetInfo(winfo);
    else
        lara.target = NULL;
    if (lara.target == NULL)
        LaraGetNewTarget(winfo);
    AimWeapon(winfo, &lara.left_arm);
    if (lara.left_arm.lock)
    {
        lara.torso_y_rot = lara.left_arm.y_rot;
        lara.torso_x_rot = lara.left_arm.x_rot;
        lara.head_x_rot = lara.head_y_rot = 0;
    }
    AnimateShotgun(weapon_type);
    if (lara.right_arm.flash_gun)
    {
        if (weapon_type == LG_SHOTGUN || weapon_type == LG_M16)
        {
            AddDynamicLight(lara_item->pos.x_pos + (phd_sin(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)),
                lara_item->pos.y_pos - WALL_L / 2,
                lara_item->pos.z_pos + (phd_cos(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)),
                12, 11);
        }
    }
}

#define PELLET_SCATTER (20 * ONE_DEGREE)
#define SHOTGUN_AMMO_CLIP 6

void FireShotgun(void)
{
    int i, r, fired;
    PHD_ANGLE angles[2];
    PHD_ANGLE dangles[2];

    angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
    angles[1] = lara.left_arm.x_rot;

    fired = 0;
    for (i = 0; i < SHOTGUN_AMMO_CLIP; i++)
    {
        r = (int)((GetRandomControl() - 16384) * PELLET_SCATTER) / 65536;
        dangles[0] = angles[0] + r;
        r = (int)((GetRandomControl() - 16384) * PELLET_SCATTER) / 65536;
        dangles[1] = angles[1] + r;
        if (FireWeapon(LG_SHOTGUN, lara.target, lara_item, dangles))
            fired = 1;
    }
    if (fired)
    {
        lara.right_arm.flash_gun = weapons[LG_SHOTGUN].flash_time;
        SoundEffect(weapons[LG_SHOTGUN].sample_num, &lara_item->pos, 0);
    }
}

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

#define HARPOON_BOLT 249
#define HARPOON_SPEED 150
#define SQUARE(A) ((A) * (A))

sint16 CreateItem(void);
void GetLaraJointAbsPosition(PHD_VECTOR *vec, int joint);
void InitialiseItem(sint16 item_num);
void find_target_point(ITEM_INFO *item, GAME_VECTOR *target);

void FireHarpoon(void)
{
    sint16 item_number;
    ITEM_INFO *item;
    GAME_VECTOR pos;
    int distance;

    if (lara.harpoon.ammo <= 0)
        return;

    item_number = CreateItem();
    if (item_number != NO_ITEM)
    {
        item = &items[item_number];

        item->object_number = HARPOON_BOLT;
        item->room_number = lara_item->room_number;
        pos.x = -2;
        pos.y = 273 + 100;
        pos.z = 77;
        GetLaraJointAbsPosition((PHD_VECTOR *)&pos, 10);
        item->pos.x_pos = pos.x;
        item->pos.y_pos = pos.y;
        item->pos.z_pos = pos.z;

        InitialiseItem(item_number);

        if (lara.target)
        {
            find_target_point(lara.target, &pos);

            item->pos.y_rot = phd_atan(pos.z - item->pos.z_pos, pos.x - item->pos.x_pos);
            distance = phd_sqrt(SQUARE(pos.z - item->pos.z_pos) + SQUARE(pos.x - item->pos.x_pos));
            item->pos.x_rot = -phd_atan(distance, pos.y - item->pos.y_pos);
        }
        else
        {
            item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
            item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
        }
        item->pos.z_rot = 0;

        item->fallspeed = (sint16)(-HARPOON_SPEED * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
        item->speed = (sint16)(HARPOON_SPEED * phd_cos(item->pos.x_rot) >> W2V_SHIFT);

        AddActiveItem(item_number);

        if (!savegame.bonus_flag)
            lara.harpoon.ammo--;
        savegame.ammo_used++;
    }
}

#define ROCKET 248

#define UNDERWATER 1
#define SMASH_WINDOW 72

sint16 *GetBestFrame(ITEM_INFO *item);
void SmashWindow(sint16 item_number);
void HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int damage);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number);

void ControlHarpoonBolt(sint16 item_number)
{
    ITEM_INFO *item, *target;
    sint16 target_number, room_number;
    sint32 old_x, sx, s, c, ox, sz, old_z, oz, rx, x, rz, z;
    sint16 *bounds;
    FLOOR_INFO *floor;

    item = &items[item_number];

    old_z = item->pos.z_pos;
    old_x = item->pos.x_pos;

    if (!(room[item->room_number].flags & UNDERWATER))
        item->fallspeed += 3;
    item->pos.y_pos += item->fallspeed;

    item->pos.z_pos += item->speed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
    item->pos.x_pos += item->speed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    for (target_number = room[item->room_number].item_number; target_number != NO_ITEM; target_number = target->next_item)
    {
        target = &items[target_number];
        if (target == lara_item || !target->collidable)
            continue;

        if (target->object_number == SMASH_WINDOW || (target->status != INVISIBLE && objects[target->object_number].collision))
        {
            bounds = GetBestFrame(target);
            if (item->pos.y_pos < target->pos.y_pos + bounds[2] || item->pos.y_pos > target->pos.y_pos + bounds[3])
                continue;

            c = phd_cos(target->pos.y_rot);
            s = phd_sin(target->pos.y_rot);

            x = item->pos.x_pos - target->pos.x_pos;
            z = item->pos.z_pos - target->pos.z_pos;
            rx = (c * x - s * z) >> W2V_SHIFT;

            ox = old_x - target->pos.x_pos;
            oz = old_z - target->pos.z_pos;
            sx = (c * ox - s * oz) >> W2V_SHIFT;

            if ((rx < bounds[0] && sx < bounds[0]) || (rx > bounds[1] && sx > bounds[1]))
                continue;

            rz = (c * z + s * x) >> W2V_SHIFT;
            sz = (c * oz + s * ox) >> W2V_SHIFT;

            if ((rz < bounds[4] && sz < bounds[4]) || (rz > bounds[5] && sz > bounds[5]))
                continue;

            if (target->object_number == SMASH_WINDOW)
                SmashWindow(target_number);
            else
            {
                if (objects[target->object_number].intelligent)
                {
                    DoLotsOfBlood(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 0, 0, item->room_number, 5);
                    HitTarget(target, NULL, weapons[LG_HARPOON].damage);
                    savegame.ammo_hit++;
                }
                KillItem(item_number);
                return;
            }
        }
    }

    if (item->pos.y_pos >= item->floor || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos))
    {
        KillItem(item_number);
        return;
    }

    if (room[item->room_number].flags & UNDERWATER)
        CreateBubble(&item->pos, item->room_number);
}

#define ROCKET 248

void FireRocket(void)
{
    sint16 item_number;
    ITEM_INFO *item;
    PHD_VECTOR pos;

    if (lara.grenade.ammo <= 0)
        return;

    item_number = CreateItem();
    if (item_number != NO_ITEM)
    {
        item = &items[item_number];

        item->object_number = ROCKET;
        item->room_number = lara_item->room_number;
        pos.x = -2;
        pos.y = 273 + 100;
        pos.z = 77;
        GetLaraJointAbsPosition(&pos, 10);
        item->pos.x_pos = pos.x;
        item->pos.y_pos = pos.y;
        item->pos.z_pos = pos.z;

        InitialiseItem(item_number);

        item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
        item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
        item->pos.z_rot = 0;
        item->speed = 200;
        item->fallspeed = 0;

        AddActiveItem(item_number);

        if (!savegame.bonus_flag)
            lara.grenade.ammo--;
        savegame.ammo_used++;
    }
}

#define EXPLOSION1 229
#define DRAGON_FRONT 22
#define BIG_YETI 46
#define ROCKET_BLAST_RADIUS 512

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
sint16 *GetBestFrame(ITEM_INFO *item);
void SmashWindow(sint16 item_number);
void HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int damage);
void CreatureDie(sint16 item_number, int explode);

void ControlRocket(sint16 item_number)
{
    sint32 sz, rz, explode, c, s, sx, speed, x, z, oz, rx, ox, radius;
    sint16 room_number, target_number;
    ITEM_INFO *item, *target;
    FX_INFO *fx;
    sint16 *bounds;
    sint32 old_x, old_z, old_y;
    FLOOR_INFO *floor;
    sint16 fx_number;

    item = &items[item_number];

    old_x = item->pos.x_pos;
    old_y = item->pos.y_pos;
    old_z = item->pos.z_pos;

    if (item->speed-- < 190)
        item->fallspeed++;

    item->pos.y_pos += item->fallspeed - (item->speed * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
    speed = item->speed * phd_cos(item->pos.x_rot) >> W2V_SHIFT;
    item->pos.z_pos += speed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
    item->pos.x_pos += speed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    if (item->pos.y_pos >= item->floor || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos))
    {
        radius = ROCKET_BLAST_RADIUS;
        explode = 1;
    }
    else
        radius = explode = 0;

    for (target_number = room[item->room_number].item_number; target_number != NO_ITEM; target_number = target->next_item)
    {
        target = &items[target_number];
        if (target == lara_item || !target->collidable)
            continue;

        if (target->object_number == SMASH_WINDOW
            || (objects[target->object_number].intelligent && target->status != INVISIBLE && objects[target->object_number].collision))
        {
            bounds = GetBestFrame(target);
            if (item->pos.y_pos + radius < target->pos.y_pos + bounds[2] || item->pos.y_pos - radius > target->pos.y_pos + bounds[3])
                continue;

            c = phd_cos(target->pos.y_rot);
            s = phd_sin(target->pos.y_rot);

            x = item->pos.x_pos - target->pos.x_pos;
            z = item->pos.z_pos - target->pos.z_pos;
            rx = (c * x - s * z) >> W2V_SHIFT;

            ox = old_x - target->pos.x_pos;
            oz = old_z - target->pos.z_pos;
            sx = (c * ox - s * oz) >> W2V_SHIFT;

            if ((rx + radius < bounds[0] && sx + radius < bounds[0]) || (rx - radius > bounds[1] && sx - radius > bounds[1]))
                continue;

            rz = (c * z + s * x) >> W2V_SHIFT;
            sz = (c * oz + s * ox) >> W2V_SHIFT;

            if ((rz + radius < bounds[4] && sz + radius < bounds[4]) || (rz - radius > bounds[5] && sz - radius > bounds[5]))
                continue;

            if (target->object_number == SMASH_WINDOW)
                SmashWindow(target_number);
            else if (target->status == ACTIVE)
            {
                HitTarget(target, NULL, 30);
                explode = 1;
                savegame.ammo_hit++;
                if (target->hit_points <= 0)
                {
                    savegame.kills++;
                    if (target->object_number != DRAGON_FRONT && target->object_number != BIG_YETI)
                        CreatureDie(target_number, 1);
                }
            }
            else
                explode = 1;
        }
    }

    if (explode)
    {
        fx_number = CreateEffect(item->room_number);
        if (fx_number != NO_ITEM)
        {
            fx = &effects[fx_number];
            fx->pos.x_pos = old_x;
            fx->pos.y_pos = old_y;
            fx->pos.z_pos = old_z;
            fx->speed = 0;
            fx->frame_number = 0;
            fx->counter = 0;
            fx->object_number = EXPLOSION1;
        }
        SoundEffect(105, NULL, 0);
        KillItem(item_number);
    }
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

void undraw_shotgun(int weapon_type)
{
    ITEM_INFO *item;

    item = &items[lara.weapon_item];
    if (lara.water_status == LARA_SURFACE)
        item->goal_anim_state = W_SURF_UNDRAW;
    else
        item->goal_anim_state = W_UNDRAW;

    AnimateItem(item);

    if (item->status == DEACTIVATED)
    {
        lara.gun_status = LG_ARMLESS;
        lara.target = NULL;
        lara.left_arm.lock = lara.right_arm.lock = 0;
        KillItem(lara.weapon_item);
        lara.weapon_item = NO_ITEM;
        lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    }
    else if (item->current_anim_state == W_UNDRAW && item->frame_number - anims[item->anim_number].frame_base == 21)
        undraw_shotgun_meshes(weapon_type);

    lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
    lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
    lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

void FireShotgun(void);

static int m16_firing;
static int harpoon_fired;

void AnimateShotgun(int weapon_type)
{
    ITEM_INFO *item;
    int running;

    item = &items[lara.weapon_item];
    if (weapon_type == LG_M16 && lara_item->speed != 0)
        running = 1;
    else
        running = 0;

    switch (item->current_anim_state)
    {
    case W_AIM:
        m16_firing = 0;
        if (harpoon_fired)
        {
            item->goal_anim_state = W_RELOAD;
            harpoon_fired = 0;
        }
        else if (lara.water_status == LARA_UNDERWATER || running)
            item->goal_anim_state = W_UAIM;
        else if (((input & IN_ACTION) && !lara.target) || lara.left_arm.lock)
            item->goal_anim_state = W_RECOIL;
        else
            item->goal_anim_state = W_UNAIM;
        break;

    case W_UAIM:
        m16_firing = 0;
        if (harpoon_fired)
        {
            item->goal_anim_state = W_RELOAD;
            harpoon_fired = 0;
        }
        else if (lara.water_status != LARA_UNDERWATER && !running)
            item->goal_anim_state = W_AIM;
        else if (((input & IN_ACTION) && !lara.target) || lara.left_arm.lock)
            item->goal_anim_state = W_URECOIL;
        else
            item->goal_anim_state = W_UUNAIM;
        break;

    case W_RECOIL:
        if (item->frame_number - anims[item->anim_number].frame_base == 0)
        {
            item->goal_anim_state = W_UNAIM;
            if (lara.water_status != LARA_UNDERWATER && !running && !harpoon_fired)
            {
                if ((input & IN_ACTION) && (!lara.target || lara.left_arm.lock))
                {
                    if (weapon_type == LG_HARPOON)
                    {
                        FireHarpoon();
                        if (!(lara.harpoon.ammo & 3))
                            harpoon_fired = 1;
                    }
                    else if (weapon_type == LG_GRENADE)
                        FireRocket();
                    else if (weapon_type == LG_M16)
                    {
                        FireM16(0);
                        SoundEffect(78, &lara_item->pos, 0);
                        m16_firing = 1;
                    }
                    else
                        FireShotgun();
                    item->goal_anim_state = W_RECOIL;
                }
                else if (lara.left_arm.lock)
                    item->goal_anim_state = W_AIM;
            }
            if (item->goal_anim_state != W_RECOIL && m16_firing)
            {
                SoundEffect(104, &lara_item->pos, 0);
                m16_firing = 0;
            }
        }
        else if (m16_firing)
            SoundEffect(78, &lara_item->pos, 0);
        else if (weapon_type == LG_SHOTGUN && !(input & IN_ACTION) && !lara.left_arm.lock)
            item->goal_anim_state = W_UNAIM;
        break;

    case W_URECOIL:
        if (item->frame_number - anims[item->anim_number].frame_base == 0)
        {
            item->goal_anim_state = W_UUNAIM;
            if ((lara.water_status == LARA_UNDERWATER || running) && !harpoon_fired)
            {
                if ((input & IN_ACTION) && (!lara.target || lara.left_arm.lock))
                {
                    if (weapon_type == LG_HARPOON)
                    {
                        FireHarpoon();
                        if (!(lara.harpoon.ammo & 3))
                            harpoon_fired = 1;
                    }
                    else
                        FireM16(1);
                    item->goal_anim_state = W_URECOIL;
                }
                else if (lara.left_arm.lock)
                    item->goal_anim_state = W_UAIM;
            }
        }
        if (weapon_type == LG_M16 && item->goal_anim_state == W_URECOIL)
            SoundEffect(78, &lara_item->pos, 0);
        break;
    }

    AnimateItem(item);

    lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
    lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
    lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}

