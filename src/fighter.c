#include "game.h"
#include "box.h"

void ShootAtLara(FX_INFO *fx);

#define KNIFE 247
#define KNIFE_SPEED 150

sint16 Knife(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    sint16 fx_number;
    FX_INFO *fx;

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
        fx->speed = KNIFE_SPEED;
        fx->frame_number = 0;
        fx->object_number = KNIFE;
        fx->shade = 14 * 256;
        ShootAtLara(fx);
    }
    return fx_number;
}

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
void CreatureHead(ITEM_INFO *item, sint16 required);
void CreatureNeck(ITEM_INFO *item, sint16 required);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
int Targetable(ITEM_INFO *item, AI_INFO *info);

#define SQUARE(A) ((A) * (A))
#define CULT2_EMPTY 0
#define CULT2_STOP 1
#define CULT2_WALK 2
#define CULT2_RUN 3
#define CULT2_AIM1L 4
#define CULT2_SHOOT1L 5
#define CULT2_AIM1R 6
#define CULT2_SHOOT1R 7
#define CULT2_AIM2 8
#define CULT2_SHOOT2 9
#define CULT2_DEATH 10
#define CULT2_DIE_ANIM 23
#define CULT2_WALK_TURN (ONE_DEGREE * 3)
#define CULT2_RUN_TURN (ONE_DEGREE * 6)
#define CULT2_WALK_RANGE SQUARE(WALL_L * 4)
#define CULT2_KNIFE_RANGE SQUARE(WALL_L * 6)
#define CULT2_STOP_RANGE SQUARE(WALL_L * 5 / 2)

extern BITE_INFO cult2_left, cult2_right;

void Cult2Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *cult;
    AI_INFO info;
    sint16 angle, head, neck, tilt;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    cult = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != CULT2_DEATH)
        {
            item->anim_number = objects[item->object_number].anim_index + CULT2_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CULT2_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, cult->maximum_turn);

        switch (item->current_anim_state)
        {
        case CULT2_STOP:
            cult->maximum_turn = 0;
            if (info.ahead)
                neck = info.angle;
            if (cult->mood == ESCAPE_MOOD)
                item->goal_anim_state = CULT2_RUN;
            else if (Targetable(item, &info))
                item->goal_anim_state = CULT2_AIM2;
            else if (cult->mood == BORED_MOOD)
            {
                if (!info.ahead || info.distance > CULT2_KNIFE_RANGE)
                    item->goal_anim_state = CULT2_WALK;
            }
            else if (info.ahead && info.distance < CULT2_WALK_RANGE)
                item->goal_anim_state = CULT2_WALK;
            else
                item->goal_anim_state = CULT2_RUN;
            break;

        case CULT2_WALK:
            cult->maximum_turn = CULT2_WALK_TURN;
            if (info.ahead)
                neck = info.angle;
            if (cult->mood == ESCAPE_MOOD)
                item->goal_anim_state = CULT2_RUN;
            else if (Targetable(item, &info))
            {
                if (info.distance < CULT2_STOP_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = CULT2_STOP;
                else if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = CULT2_AIM1L;
                else
                    item->goal_anim_state = CULT2_AIM1R;
            }
            else if (cult->mood == BORED_MOOD)
            {
                if (info.ahead && info.distance < CULT2_KNIFE_RANGE)
                    item->goal_anim_state = CULT2_STOP;
            }
            else if (!info.ahead || info.distance > CULT2_WALK_RANGE)
                item->goal_anim_state = CULT2_RUN;
            break;

        case CULT2_RUN:
            cult->maximum_turn = CULT2_RUN_TURN;
            tilt = angle >> 2;
            if (info.ahead)
                neck = info.angle;
            if (cult->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = CULT2_WALK;
            else if (cult->mood == BORED_MOOD)
            {
                if (info.ahead && info.distance < CULT2_KNIFE_RANGE)
                    item->goal_anim_state = CULT2_STOP;
                else
                    item->goal_anim_state = CULT2_WALK;
            }
            else if (info.ahead && info.distance < CULT2_WALK_RANGE)
                item->goal_anim_state = CULT2_WALK;
            break;

        case CULT2_AIM1L:
            cult->flags = 0;
            if (info.ahead)
                head = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT2_SHOOT1L;
            else
                item->goal_anim_state = CULT2_WALK;
            break;

        case CULT2_AIM1R:
            cult->flags = 0;
            if (info.ahead)
                head = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT2_SHOOT1R;
            else
                item->goal_anim_state = CULT2_WALK;
            break;

        case CULT2_AIM2:
            cult->flags = 0;
            if (info.ahead)
                head = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = CULT2_SHOOT2;
            else
                item->goal_anim_state = CULT2_STOP;
            break;

        case CULT2_SHOOT1L:
            if (info.ahead)
                head = info.angle;
            if (!cult->flags)
            {
                CreatureEffect(item, &cult2_left, Knife);
                cult->flags = 1;
            }
            break;

        case CULT2_SHOOT1R:
            if (info.ahead)
                head = info.angle;
            if (!cult->flags)
            {
                CreatureEffect(item, &cult2_right, Knife);
                cult->flags = 1;
            }
            break;

        case CULT2_SHOOT2:
            if (info.ahead)
                head = info.angle;
            if (!cult->flags)
            {
                CreatureEffect(item, &cult2_left, Knife);
                CreatureEffect(item, &cult2_right, Knife);
                cult->flags = 1;
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureNeck(item, neck);
    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, 0);
}
