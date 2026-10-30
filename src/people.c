#include "game.h"
#include "box.h"

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number);
void Richochet(GAME_VECTOR *pos);

#define SQUARE(A) ((A) * (A))
#define PEOPLE_SHOOT_RANGE SQUARE(WALL_L * 8)

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

int Targetable(ITEM_INFO *item, AI_INFO *info)
{
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    if (enemy->hit_points <= 0 || !info->ahead || info->distance >= PEOPLE_SHOOT_RANGE)
        return 0;

    start.x = item->pos.x_pos;
    start.y = item->pos.y_pos - STEP_L * 3;
    start.z = item->pos.z_pos;
    start.room_number = item->room_number;

    target.x = enemy->pos.x_pos;
    target.y = enemy->pos.y_pos - STEP_L * 3;
    target.z = enemy->pos.z_pos;

    return LOS(&start, &target);
}

void KillEffect(sint16 fx_num);

void ControlGlow(sint16 fx_number)
{
    FX_INFO *fx;

    fx = &effects[fx_number];
    if (--fx->counter)
    {
        fx->shade += fx->speed;
        fx->frame_number += fx->fallspeed;
    }
    else
        KillEffect(fx_number);
}

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

void ControlGunShot(sint16 fx_number)
{
    FX_INFO *fx;

    fx = &effects[fx_number];
    fx->counter--;

    if (!fx->counter)
    {
        KillEffect(fx_number);
        return;
    }

    fx->pos.z_rot = (sint16)GetRandomControl();
    AddDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 12, 11);
}

#define GUN_FLASH 240

sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    FX_INFO *fx;
    sint16 fx_number;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->room_number = room_number;
        fx->pos.x_rot = fx->pos.z_rot = 0;
        fx->pos.y_rot = yrot;
        fx->counter = 3;
        fx->frame_number = 0;
        fx->object_number = GUN_FLASH;
        fx->shade = 16 * 256;
    }

    return fx_number;
}

sint16 GunHit(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    PHD_VECTOR pos;

    pos.x = pos.y = pos.z = 0;
    GetJointAbsPosition(lara_item, &pos, GetRandomControl() * 25 / 0x7fff);
    DoBloodSplat(pos.x, pos.y, pos.z, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
    SoundEffect(50, &lara_item->pos, 0);

    return (GunShot(x, y, z, speed, yrot, room_number));
}

sint16 GunMiss(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    GAME_VECTOR pos;

    pos.x = lara_item->pos.x_pos + (GetRandomDraw() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.y = lara_item->floor;
    pos.z = lara_item->pos.z_pos + (GetRandomDraw() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.room_number = lara_item->room_number;
    Richochet(&pos);

    return (GunShot(x, y, z, speed, yrot, room_number));
}

#define CULT1C 18

#define PEOPLE_HIT_CHANCE 0x2000
#define PEOPLE_TARGETING_SPEED 300

int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target);
void SmashItem(sint16 item_number, int weapon_type);

int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage)
{
    int hit, targetable, random, distance;
    sint16 fx_number, smash_item;
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;

    if (info->distance > PEOPLE_SHOOT_RANGE || !Targetable(item, info))
        hit = targetable = 0;
    else
    {
        distance = (phd_sin(info->enemy_facing) * enemy->speed >> W2V_SHIFT) * PEOPLE_SHOOT_RANGE / PEOPLE_TARGETING_SPEED;
        distance = info->distance + SQUARE(distance);
        if (distance > PEOPLE_SHOOT_RANGE)
            hit = 0;
        else
        {
            random = (PEOPLE_SHOOT_RANGE - info->distance) / (PEOPLE_SHOOT_RANGE / 0x5000) + PEOPLE_HIT_CHANCE;
            hit = (GetRandomControl() < random);
        }
        targetable = 1;
    }

    fx_number = NO_ITEM;
    if (enemy == lara_item)
    {
        if (hit)
        {
            fx_number = CreatureEffect(item, gun, GunHit);
            lara_item->hit_points -= damage;
            lara_item->hit_status = 1;
        }
        else if (targetable)
            fx_number = CreatureEffect(item, gun, GunMiss);
    }
    else
    {
        fx_number = CreatureEffect(item, gun, GunShot);
        if (hit)
        {
            enemy->hit_points -= damage / 10;
            enemy->hit_status = 1;
        }
    }

    if (fx_number != NO_ITEM)
        effects[fx_number].pos.y_rot += extra_rotation;

    start.x = item->pos.x_pos;
    start.y = item->pos.y_pos - STEP_L * 3;
    start.z = item->pos.z_pos;
    start.room_number = item->room_number;

    target.x = enemy->pos.x_pos;
    target.y = enemy->pos.y_pos - STEP_L * 3;
    target.z = enemy->pos.z_pos;

    smash_item = ObjectOnLOS(&start, &target);
    if (smash_item != NO_ITEM)
        SmashItem(smash_item, 0);

    return targetable;
}

void InitialiseCult1(sint16 item_number)
{
    if (GetRandomControl() < 0x4000)
        items[item_number].mesh_bits -= 0x30;
    if (items[item_number].object_number == CULT1C)
        items[item_number].mesh_bits -= 0x1f8000;
}

#define CULT3 20
#define CULT3_WAIT 2
#define CULT3_WAIT_ANIM 3

void InitialiseCreature(sint16 item_number);

void InitialiseCult3(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[CULT3].anim_index + CULT3_WAIT_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = CULT3_WAIT;
}
