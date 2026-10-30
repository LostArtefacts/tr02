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

#define CULT3_STOP 1
#define CULT3_WALK 3
#define CULT3_RUN 4
#define CULT3_AIML 5
#define CULT3_AIMR 6
#define CULT3_SHOOTL 7
#define CULT3_SHOOTR 8
#define CULT3_AIM2 9
#define CULT3_SHOOT2 10
#define CULT3_DEATH 11
#define CULT3_SHOT_DAMAGE 50
#define CULT3_RUN_TURN (ONE_DEGREE * 3)
#define CULT3_WALK_TURN (ONE_DEGREE * 3)
#define CULT3_STOP_RANGE SQUARE(WALL_L * 3)
#define CULT3_RUN_RANGE SQUARE(WALL_L * 5)
#define CULT3_DIE_ANIM 32
#define ROT_Y 8

extern BITE_INFO cult3_gunl, cult3_gunr;

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
void CreatureHead(ITEM_INFO *item, sint16 required);
void CreatureNeck(ITEM_INFO *item, sint16 required);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

void Cult3Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *cult;
    sint16 angle, tilt;
    sint16 head, left, right, body;
    AI_INFO info;
    OBJECT_INFO *obj;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    cult = (CREATURE_INFO *)item->data;
    angle = tilt = 0;
    head = left = right = body = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != CULT3_DEATH)
        {
            item->anim_number = objects[CULT3].anim_index + CULT3_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CULT3_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, cult->maximum_turn);

        switch (item->current_anim_state)
        {
        case CULT3_WAIT:
        case CULT3_STOP:
            if (info.ahead)
                head = info.angle;
            if (cult->mood == BORED_MOOD && lara_item->hit_points <= 0)
                item->goal_anim_state = CULT3_WAIT;
            else if (Targetable(item, &info))
            {
                if (info.distance > CULT3_STOP_RANGE)
                    item->goal_anim_state = CULT3_WALK;
                else
                    item->goal_anim_state = CULT3_AIM2;
            }
            else if (cult->mood == ESCAPE_MOOD)
                item->goal_anim_state = CULT3_RUN;
            else if (cult->mood == ATTACK_MOOD)
            {
                if (info.distance > CULT3_RUN_RANGE || !info.ahead)
                    item->goal_anim_state = CULT3_RUN;
                else
                    item->goal_anim_state = CULT3_WALK;
            }
            else if (cult->mood == STALK_MOOD || !info.ahead)
                item->goal_anim_state = CULT3_WALK;
            break;

        case CULT3_WALK:
            cult->maximum_turn = CULT3_WALK_TURN;
            if (info.ahead)
                head = info.angle;
            if (Targetable(item, &info))
            {
                if (info.distance < CULT3_STOP_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = CULT3_STOP;
                else if (info.angle < 0)
                    item->goal_anim_state = CULT3_AIML;
                else
                    item->goal_anim_state = CULT3_AIMR;
            }
            else if (cult->mood == ESCAPE_MOOD)
                item->goal_anim_state = CULT3_RUN;
            else if (cult->mood == STALK_MOOD || cult->mood == ATTACK_MOOD)
            {
                if (info.distance > CULT3_RUN_RANGE || !info.ahead)
                    item->goal_anim_state = CULT3_RUN;
            }
            else if (lara_item->hit_points <= 0)
                item->goal_anim_state = CULT3_WAIT;
            else if (info.ahead)
                item->goal_anim_state = CULT3_STOP;
            break;

        case CULT3_RUN:
            cult->maximum_turn = CULT3_RUN_TURN;
            tilt = angle / 4;
            if (info.ahead)
                head = info.angle;
            if (Targetable(item, &info))
            {
                if (info.zone_number != info.enemy_zone)
                    item->goal_anim_state = CULT3_STOP;
                else if (info.angle < 0)
                    item->goal_anim_state = CULT3_AIML;
                else
                    item->goal_anim_state = CULT3_AIMR;
            }
            else if (cult->mood == BORED_MOOD)
            {
                if (lara_item->hit_points <= 0)
                    item->goal_anim_state = CULT3_WAIT;
                else
                    item->goal_anim_state = CULT3_STOP;
            }
            else if (info.ahead && info.distance < CULT3_RUN_RANGE)
                item->goal_anim_state = CULT3_WALK;
            break;

        case CULT3_AIM2:
            cult->flags = 0;
            if (info.ahead)
                body = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT3_SHOOT2;
            else
                item->goal_anim_state = CULT3_STOP;
            break;

        case CULT3_AIML:
            cult->flags = 0;
            if (info.ahead)
                left = head = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT3_SHOOTL;
            else
                item->goal_anim_state = CULT3_WALK;
            break;

        case CULT3_AIMR:
            cult->flags = 0;
            if (info.ahead)
                right = head = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT3_SHOOTR;
            else
                item->goal_anim_state = CULT3_WALK;
            break;

        case CULT3_SHOOT2:
            if (info.ahead)
                body = info.angle;
            if (!cult->flags)
            {
                ShotLara(item, &info, &cult3_gunl, head, CULT3_SHOT_DAMAGE);
                ShotLara(item, &info, &cult3_gunr, head, CULT3_SHOT_DAMAGE);
                cult->flags = 1;
            }
            break;

        case CULT3_SHOOTL:
            if (info.ahead)
                left = head = info.angle;
            if (!cult->flags)
            {
                ShotLara(item, &info, &cult3_gunl, head, CULT3_SHOT_DAMAGE);
                cult->flags = 1;
            }
            break;

        case CULT3_SHOOTR:
            if (info.ahead)
                right = head = info.angle;
            if (!cult->flags)
            {
                ShotLara(item, &info, &cult3_gunr, head, CULT3_SHOT_DAMAGE);
                cult->flags = 1;
            }
            break;
        }
    }

    CreatureTilt(item, tilt);

    obj = &objects[CULT3];
    if (body)
        bones[obj->bone_index] |= ROT_Y;
    else
        bones[obj->bone_index] &= ~ROT_Y;
    if (left)
        bones[obj->bone_index + 2 * 4] |= ROT_Y;
    else
        bones[obj->bone_index + 2 * 4] &= ~ROT_Y;
    if (right)
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
    else
        bones[obj->bone_index + 6 * 4] &= ~ROT_Y;
    if (head)
        bones[obj->bone_index + 10 * 4] |= ROT_Y;
    else
        bones[obj->bone_index + 10 * 4] &= ~ROT_Y;

    if (body)
        CreatureHead(item, body);
    else if (left)
    {
        CreatureHead(item, left);
        CreatureNeck(item, head);
    }
    else if (right)
    {
        CreatureHead(item, right);
        CreatureNeck(item, head);
    }
    else if (head)
        CreatureHead(item, head);

    CreatureAnimation(item_number, angle, 0);
}


#define WORKER1 30
#define WORK1_SHOT_DAMAGE 150
#define WORK1_WALK_TURN (ONE_DEGREE * 3)
#define WORK1_RUN_TURN (ONE_DEGREE * 5)
#define WORK1_RUN_RANGE SQUARE(WALL_L * 2)
#define WORK1_SHOOT1_RANGE SQUARE(WALL_L * 3)
#define WORK1_DIE_ANIM 18

enum work1_anims {
    WORK1_EMPTY,
    WORK1_WALK,
    WORK1_STOP,
    WORK1_WAIT,
    WORK1_SHOOT1,
    WORK1_RUN,
    WORK1_SHOOT2,
    WORK1_DEATH,
    WORK1_AIM,
    WORK1_AIM2,
    WORK1_SHOOT3
};

extern BITE_INFO work1_gun;

void Worker1Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *worker;
    sint16 angle, head, neck, tilt;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    worker = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != WORK1_DEATH)
        {
            item->anim_number = objects[WORKER1].anim_index + WORK1_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK1_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, worker->maximum_turn);

        switch (item->current_anim_state)
        {
        case WORK1_STOP:
            if (info.ahead)
                neck = info.angle;
            worker->flags = 0;
            worker->maximum_turn = 0;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK1_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < WORK1_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = WORK1_AIM;
                    else
                        item->goal_anim_state = WORK1_AIM2;
                }
                else
                    item->goal_anim_state = WORK1_WALK;
            }
            else if (worker->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = WORK1_WAIT;
            else if (info.distance > WORK1_RUN_RANGE)
                item->goal_anim_state = WORK1_RUN;
            else
                item->goal_anim_state = WORK1_WALK;
            break;

        case WORK1_WAIT:
            if (info.ahead)
                neck = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = WORK1_SHOOT1;
            else if (worker->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = WORK1_STOP;
            break;

        case WORK1_WALK:
            if (info.ahead)
                neck = info.angle;
            worker->flags = 0;
            worker->maximum_turn = WORK1_WALK_TURN;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK1_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < WORK1_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = WORK1_STOP;
                else
                    item->goal_anim_state = WORK1_SHOOT2;
            }
            else if (worker->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = WORK1_STOP;
            else if (info.distance > WORK1_RUN_RANGE)
                item->goal_anim_state = WORK1_RUN;
            break;

        case WORK1_RUN:
            if (info.ahead)
                neck = info.angle;
            worker->maximum_turn = WORK1_RUN_TURN;
            tilt = angle / 2;
            if (worker->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = WORK1_WALK;
            else if (worker->mood == BORED_MOOD || worker->mood == STALK_MOOD)
                item->goal_anim_state = WORK1_WALK;
            break;

        case WORK1_AIM2:
            if (Targetable(item, &info))
                item->goal_anim_state = WORK1_SHOOT3;
        case WORK1_AIM:
            worker->flags = 0;
            if (info.ahead)
                head = info.angle;
            break;

        case WORK1_SHOOT1:
        case WORK1_SHOOT3:
            if (info.ahead)
                head = info.angle;
            if (!worker->flags)
            {
                ShotLara(item, &info, &work1_gun, head, WORK1_SHOT_DAMAGE);
                worker->flags = 1;
            }
            if (item->goal_anim_state != WORK1_STOP)
            {
                if (worker->mood == ESCAPE_MOOD || info.distance > WORK1_SHOOT1_RANGE || !Targetable(item, &info))
                    item->goal_anim_state = WORK1_STOP;
            }
            break;

        case WORK1_SHOOT2:
            if (info.ahead)
                head = info.angle;
            if (!worker->flags)
            {
                ShotLara(item, &info, &work1_gun, head, WORK1_SHOT_DAMAGE);
                worker->flags = 1;
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);
}


#define WORKER2 31
#define WORKER4 34
#define WORK2_SHOT_DAMAGE 30
#define WORK2_WALK_TURN (ONE_DEGREE * 3)
#define WORK2_RUN_TURN (ONE_DEGREE * 5)
#define WORK2_RUN_RANGE SQUARE(WALL_L * 2)
#define WORK2_SHOOT1_RANGE SQUARE(WALL_L * 3)
#define WORK2_DIE_ANIM 19

enum work2_anims {
    WORK2_EMPTY,
    WORK2_STOP,
    WORK2_WALK,
    WORK2_RUN,
    WORK2_WAIT,
    WORK2_SHOOT1,
    WORK2_SHOOT2,
    WORK2_DEATH,
    WORK2_AIM1,
    WORK2_AIM2,
    WORK2_AIM3,
    WORK2_SHOOT3
};

extern BITE_INFO work2_gun;

sint16 DragonFire(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number);

void Worker2Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *worker;
    sint16 angle, head, neck, tilt;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    worker = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != WORK2_DEATH)
        {
            item->anim_number = objects[item->object_number].anim_index + WORK2_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK2_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, worker->maximum_turn);

        switch (item->current_anim_state)
        {
        case WORK2_STOP:
            if (info.ahead)
                neck = info.angle;
            worker->flags = 0;
            worker->maximum_turn = 0;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK2_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < WORK2_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                {
                    if (item->object_number == WORKER4 || GetRandomControl() < 0x4000)
                        item->goal_anim_state = WORK2_AIM1;
                    else
                        item->goal_anim_state = WORK2_AIM3;
                }
                else
                    item->goal_anim_state = WORK2_WALK;
            }
            else if (worker->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = WORK2_WAIT;
            else if (info.distance > WORK2_RUN_RANGE)
                item->goal_anim_state = WORK2_RUN;
            else
                item->goal_anim_state = WORK2_WALK;
            break;

        case WORK2_WAIT:
            if (info.ahead)
                neck = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = WORK2_SHOOT1;
            else if (worker->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = WORK2_STOP;
            break;

        case WORK2_WALK:
            if (info.ahead)
                neck = info.angle;
            worker->flags = 0;
            worker->maximum_turn = WORK2_WALK_TURN;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK2_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < WORK2_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = WORK2_STOP;
                else
                    item->goal_anim_state = WORK2_AIM2;
            }
            else if (worker->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = WORK2_STOP;
            else if (info.distance > WORK2_RUN_RANGE)
                item->goal_anim_state = WORK2_RUN;
            break;

        case WORK2_RUN:
            if (info.ahead)
                neck = info.angle;
            worker->maximum_turn = WORK2_RUN_TURN;
            tilt = angle / 2;
            if (worker->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = WORK2_WALK;
            else if (worker->mood == BORED_MOOD || worker->mood == STALK_MOOD)
                item->goal_anim_state = WORK2_WALK;
            break;

        case WORK2_AIM1:
        case WORK2_AIM3:
            worker->flags = 0;
            if (info.ahead)
            {
                head = info.angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = (item->current_anim_state == WORK2_AIM1) ? WORK2_SHOOT1 : WORK2_SHOOT3;
                else
                    item->goal_anim_state = WORK2_STOP;
            }
            break;

        case WORK2_AIM2:
            worker->flags = 0;
            if (info.ahead)
            {
                head = info.angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = WORK2_SHOOT2;
                else
                    item->goal_anim_state = WORK2_WALK;
            }
            break;

        case WORK2_SHOOT3:
            if (item->goal_anim_state != WORK2_STOP)
            {
                if (worker->mood == ESCAPE_MOOD || info.distance > WORK2_SHOOT1_RANGE || !Targetable(item, &info))
                    item->goal_anim_state = WORK2_STOP;
            }
        case WORK2_SHOOT2:
        case WORK2_SHOOT1:
            if (info.ahead)
                head = info.angle;
            if (item->object_number == WORKER2)
            {
                if (!worker->flags)
                {
                    ShotLara(item, &info, &work2_gun, head, WORK2_SHOT_DAMAGE);
                    worker->flags = 5;
                }
                else
                    worker->flags--;
            }
            else
                CreatureEffect(item, &work2_gun, DragonFire);
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);
}


#define BANDIT1 48
#define BANDIT_SHOT_DAMAGE 8
#define BANDIT_WALK_TURN (ONE_DEGREE * 4)
#define BANDIT_RUN_TURN (ONE_DEGREE * 6)
#define BANDIT_WALK_RANGE SQUARE(WALL_L * 2)
#define BANDIT_DIE_ANIM 14

enum bandit_anims {
    BANDIT_EMPTY,
    BANDIT_WAIT,
    BANDIT_WALK,
    BANDIT_RUN,
    BANDIT_AIM1,
    BANDIT_SHOOT1,
    BANDIT_AIM2,
    BANDIT_SHOOT2,
    BANDIT_SHOOT3A,
    BANDIT_SHOOT3B,
    BANDIT_SHOOT4A,
    BANDIT_AIM3,
    BANDIT_AIM4,
    BANDIT_DEATH,
    BANDIT_SHOOT4B
};

extern BITE_INFO bandit_gun;

void BanditControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *bandit;
    sint16 angle, head, neck, tilt;
    int random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    bandit = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        item->hit_points = 0;
        if (item->current_anim_state != BANDIT_DEATH)
        {
            item->anim_number = objects[BANDIT1].anim_index + BANDIT_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BANDIT_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, bandit->maximum_turn);

        switch (item->current_anim_state)
        {
        case BANDIT_WAIT:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = 0;
            if (bandit->mood == ESCAPE_MOOD)
                item->goal_anim_state = BANDIT_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance > BANDIT_WALK_RANGE)
                    item->goal_anim_state = BANDIT_WALK;
                else
                {
                    random = GetRandomControl();
                    if (random < 0x2000)
                        item->goal_anim_state = BANDIT_SHOOT1;
                    else if (random < 0x4000)
                        item->goal_anim_state = BANDIT_SHOOT2;
                    else
                        item->goal_anim_state = BANDIT_AIM3;
                }
            }
            else if (bandit->mood == BORED_MOOD)
            {
                if (info.ahead)
                    item->goal_anim_state = BANDIT_WAIT;
                else
                    item->goal_anim_state = BANDIT_WALK;
            }
            else
                item->goal_anim_state = BANDIT_RUN;
            break;

        case BANDIT_WALK:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = BANDIT_WALK_TURN;
            if (bandit->mood == ESCAPE_MOOD)
                item->goal_anim_state = BANDIT_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance > BANDIT_WALK_RANGE && info.zone_number == info.enemy_zone)
                    item->goal_anim_state = BANDIT_AIM4;
                else
                    item->goal_anim_state = BANDIT_WAIT;
            }
            else if (bandit->mood == BORED_MOOD)
            {
                if (info.ahead)
                    item->goal_anim_state = BANDIT_WALK;
                else
                    item->goal_anim_state = BANDIT_WAIT;
            }
            else
                item->goal_anim_state = BANDIT_RUN;
            break;

        case BANDIT_RUN:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = BANDIT_RUN_TURN;
            tilt = angle / 2;
            if (bandit->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = BANDIT_WAIT;
            else if (bandit->mood == BORED_MOOD)
                item->goal_anim_state = BANDIT_WALK;
            break;

        case BANDIT_SHOOT1:
        case BANDIT_SHOOT2:
        case BANDIT_SHOOT3A:
        case BANDIT_SHOOT3B:
            if (info.ahead)
                head = info.angle;
            if (!ShotLara(item, &info, &bandit_gun, head, BANDIT_SHOT_DAMAGE))
                item->goal_anim_state = BANDIT_WAIT;
            break;

        case BANDIT_SHOOT4A:
        case BANDIT_SHOOT4B:
            if (info.ahead)
                head = info.angle;
            if (!ShotLara(item, &info, &bandit_gun, head, BANDIT_SHOT_DAMAGE))
                item->goal_anim_state = BANDIT_WALK;
            if (info.distance < BANDIT_WALK_RANGE)
                item->goal_anim_state = BANDIT_WALK;
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);
}


#define BANDIT2 49
#define BANDIT2_SHOT_DAMAGE 50
#define BANDIT2_WALK_TURN (ONE_DEGREE * 4)
#define BANDIT2_RUN_TURN (ONE_DEGREE * 6)
#define BANDIT2_WALK_RANGE SQUARE(WALL_L * 2)
#define BANDIT2_DIE_ANIM 9

enum bandit2_anims {
    BANDIT2_EMPTY,
    BANDIT2_AIM4,
    BANDIT2_WAIT,
    BANDIT2_WALK,
    BANDIT2_RUN,
    BANDIT2_AIM1,
    BANDIT2_AIM2,
    BANDIT2_SHOOT1,
    BANDIT2_SHOOT2,
    BANDIT2_SHOOT4A,
    BANDIT2_SHOOT4B,
    BANDIT2_DEATH,
    BANDIT2_AIM5,
    BANDIT2_SHOOT5
};

extern BITE_INFO bandit2_gun;

void Bandit2Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *bandit;
    sint16 angle, head, neck, tilt;
    int random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    bandit = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != BANDIT2_DEATH)
        {
            item->anim_number = objects[BANDIT2].anim_index + BANDIT2_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BANDIT2_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, bandit->maximum_turn);

        switch (item->current_anim_state)
        {
        case BANDIT2_WAIT:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = 0;
            if (bandit->mood == ESCAPE_MOOD)
                item->goal_anim_state = BANDIT2_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance > BANDIT2_WALK_RANGE && GetRandomControl() < 0x4000)
                    item->goal_anim_state = BANDIT2_WALK;
                else
                {
                    random = GetRandomControl();
                    if (random < 0x2000)
                        item->goal_anim_state = BANDIT2_SHOOT1;
                    else if (random < 0x5000)
                        item->goal_anim_state = BANDIT2_SHOOT2;
                    else
                        item->goal_anim_state = BANDIT2_AIM5;
                }
            }
            else if (bandit->mood == BORED_MOOD)
            {
                if (!info.ahead || GetRandomControl() < 0x100)
                    item->goal_anim_state = BANDIT2_WALK;
            }
            else
                item->goal_anim_state = BANDIT2_RUN;
            break;

        case BANDIT2_WALK:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = BANDIT2_WALK_TURN;
            if (bandit->mood == ESCAPE_MOOD)
                item->goal_anim_state = BANDIT2_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < BANDIT2_WALK_RANGE || info.zone_number == info.enemy_zone || GetRandomControl() < 0x400)
                    item->goal_anim_state = BANDIT2_WAIT;
                else
                    item->goal_anim_state = BANDIT2_AIM4;
            }
            else if (bandit->mood == BORED_MOOD)
            {
                if (info.ahead && GetRandomControl() < 0x400)
                    item->goal_anim_state = BANDIT2_WAIT;
            }
            else
                item->goal_anim_state = BANDIT2_RUN;
            break;

        case BANDIT2_RUN:
            if (info.ahead)
                neck = info.angle;
            bandit->maximum_turn = BANDIT2_RUN_TURN;
            tilt = angle / 2;
            if (bandit->mood == ESCAPE_MOOD)
                break;
            else if (bandit->mood == BORED_MOOD || Targetable(item, &info))
                item->goal_anim_state = BANDIT2_WAIT;
            break;

        case BANDIT2_AIM1:
        case BANDIT2_AIM2:
        case BANDIT2_AIM4:
            if (info.ahead)
                head = info.angle;
            bandit->flags = 0;
            break;

        case BANDIT2_AIM5:
            if (info.ahead)
                head = info.angle;
            bandit->flags = 0;
            if (Targetable(item, &info))
                item->goal_anim_state = BANDIT2_SHOOT5;
            else
                item->goal_anim_state = BANDIT2_WAIT;
            break;

        case BANDIT2_SHOOT1:
        case BANDIT2_SHOOT2:
        case BANDIT2_SHOOT5:
            if (info.ahead)
                head = info.angle;
            if (!bandit->flags)
            {
                if (!ShotLara(item, &info, &bandit2_gun, head, BANDIT2_SHOT_DAMAGE) || GetRandomControl() < 0x2000)
                    item->goal_anim_state = BANDIT2_WAIT;
                bandit->flags = 1;
            }
            break;

        case BANDIT2_SHOOT4A:
            if (info.ahead)
                head = info.angle;
            if (bandit->flags != 1)
            {
                if (!ShotLara(item, &info, &bandit2_gun, head, BANDIT2_SHOT_DAMAGE))
                    item->goal_anim_state = BANDIT2_WALK;
                bandit->flags = 1;
            }
            if (info.distance < BANDIT2_WALK_RANGE)
                item->goal_anim_state = BANDIT2_WALK;
            break;

        case BANDIT2_SHOOT4B:
            if (info.ahead)
                head = info.angle;
            if (bandit->flags != 2)
            {
                if (!ShotLara(item, &info, &bandit2_gun, head, BANDIT2_SHOT_DAMAGE))
                    item->goal_anim_state = BANDIT2_WALK;
                bandit->flags = 2;
            }
            if (info.distance < BANDIT2_WALK_RANGE)
                item->goal_anim_state = BANDIT2_WALK;
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);
}


#define WINSTON_STOP 1
#define WINSTON_WALK 2
#define WINSTON_STOP_RANGE SQUARE(WALL_L * 3 / 2)

void WinstonControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *winston;
    sint16 angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    winston = (CREATURE_INFO *)item->data;

    CreatureAIInfo(item, &info);
    CreatureMood(item, &info, 1);
    angle = CreatureTurn(item, winston->maximum_turn);

    if (item->current_anim_state == WINSTON_STOP)
    {
        if (info.distance > WINSTON_STOP_RANGE || !info.ahead)
        {
            if (item->goal_anim_state != WINSTON_WALK)
            {
                item->goal_anim_state = WINSTON_WALK;
                SoundEffect(345, &item->pos, 0);
            }
        }
    }
    else
    {
        if (info.distance < WINSTON_STOP_RANGE)
        {
            if (info.ahead)
            {
                item->goal_anim_state = WINSTON_STOP;
                if (winston->flags & 1)
                    winston->flags--;
            }
            else if (!(winston->flags & 1))
            {
                SoundEffect(344, &item->pos, 0);
                SoundEffect(347, &item->pos, 0);
                winston->flags |= 1;
            }
        }
    }

    if (item->touch_bits && !(winston->flags & 2))
    {
        SoundEffect(346, &item->pos, 0);
        SoundEffect(347, &item->pos, 0);
        winston->flags |= 2;
    }
    else if (!item->touch_bits && (winston->flags & 2))
        winston->flags -= 2;

    if (GetRandomDraw() < 0x100)
        SoundEffect(347, &item->pos, 0);

    CreatureAnimation(item_number, angle, 0);
}
