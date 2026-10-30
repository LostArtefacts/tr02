#include "game.h"
#include "box.h"

int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

#define SPIDER 36
#define SPIDER_EMPTY 0
#define SPIDER_STOP 1
#define SPIDER_WALK1 2
#define SPIDER_WALK2 3
#define SPIDER_ATTACK1 4
#define SPIDER_ATTACK2 5
#define SPIDER_ATTACK3 6
#define SPIDER_DEATH 7
#define SPIDER_LEAP_ANIM 2

void SpiderLeap(sint16 item_number, sint16 angle)
{
    ITEM_INFO *item;
    sint32 x, y, z;
    sint16 room_number;

    item = &items[item_number];
    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    room_number = item->room_number;

    CreatureAnimation(item_number, angle, 0);

    if (item->pos.y_pos > y - STEP_L * 3 / 2)
        return;

    item->pos.x_pos = x;
    item->pos.y_pos = y;
    item->pos.z_pos = z;
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    item->anim_number = objects[SPIDER].anim_index + SPIDER_LEAP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = SPIDER_ATTACK2;

    CreatureAnimation(item_number, angle, 0);
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
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
void DisableBaddieAI(sint16 item_number);

#define SQUARE(A) ((A) * (A))
#define SPIDER_DAMAGE 25
#define SPIDER_WALK_TURN (ONE_DEGREE * 8)
#define SPIDER_MOVE_CHANCE 0x100
#define SPIDER_ATTACK3_RANGE SQUARE(WALL_L / 5)
#define SPIDER_ATTACK2_RANGE SQUARE(WALL_L / 2)

extern BITE_INFO spider_bite;

void SpiderControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *spider;
    sint16 angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    spider = (CREATURE_INFO *)item->data;
    angle = 0;

    if (item->hit_points <= 0)
    {
        if (ExplodingDeath(item_number, 0xffffffff, 0))
        {
            DisableBaddieAI(item_number);
            KillItem(item_number);
            item->status = DEACTIVATED;
            SoundEffect(349, &item->pos, 0);
            return;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, SPIDER_WALK_TURN);

        switch (item->current_anim_state)
        {
        case SPIDER_STOP:
            spider->flags = 0;
            if (spider->mood == BORED_MOOD)
            {
                if (GetRandomControl() < SPIDER_MOVE_CHANCE)
                    item->goal_anim_state = SPIDER_WALK1;
                else
                    break;
            }
            else if (info.ahead && item->touch_bits)
                item->goal_anim_state = SPIDER_ATTACK1;
            else if (spider->mood == STALK_MOOD)
                item->goal_anim_state = SPIDER_WALK1;
            else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
                item->goal_anim_state = SPIDER_WALK2;
            break;

        case SPIDER_WALK1:
            if (spider->mood == BORED_MOOD)
            {
                if (GetRandomControl() < SPIDER_MOVE_CHANCE)
                    item->goal_anim_state = SPIDER_STOP;
                else
                    break;
            }
            else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
                item->goal_anim_state = SPIDER_WALK2;
            break;

        case SPIDER_WALK2:
            spider->flags = 0;
            if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
                item->goal_anim_state = SPIDER_WALK1;
            else if (info.ahead && item->touch_bits)
                item->goal_anim_state = SPIDER_STOP;
            else if (info.ahead && info.distance < SPIDER_ATTACK3_RANGE)
                item->goal_anim_state = SPIDER_ATTACK3;
            else if (info.ahead && info.distance < SPIDER_ATTACK2_RANGE)
                item->goal_anim_state = SPIDER_ATTACK2;
            break;

        case SPIDER_ATTACK1:
        case SPIDER_ATTACK2:
        case SPIDER_ATTACK3:
            if (!spider->flags && item->touch_bits)
            {
                CreatureEffect(item, &spider_bite, DoBloodSplat);
                lara_item->hit_points -= SPIDER_DAMAGE;
                lara_item->hit_status = 1;
                spider->flags = 1;
            }
            break;
        }
    }

    if (item->current_anim_state == SPIDER_ATTACK2 && item->current_anim_state == SPIDER_ATTACK1)
        CreatureAnimation(item_number, angle, 0);
    else
        SpiderLeap(item_number, angle);
}

#define BIG_SPIDER 37
#define BIG_SPIDER_DAMAGE 100
#define BIG_SPIDER_DIE_ANIM 2
#define BIG_SPIDER_TURN (ONE_DEGREE * 4)

void BigSpiderControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *spider;
    sint16 angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    spider = (CREATURE_INFO *)item->data;
    angle = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != SPIDER_DEATH)
        {
            item->anim_number = objects[BIG_SPIDER].anim_index + BIG_SPIDER_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = SPIDER_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);

        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, BIG_SPIDER_TURN);

        switch (item->current_anim_state)
        {
        case SPIDER_STOP:
            spider->flags = 0;
            if (spider->mood == BORED_MOOD)
                break;
            else if (info.ahead && item->touch_bits)
                item->goal_anim_state = SPIDER_ATTACK1;
            else if (spider->mood == STALK_MOOD)
                item->goal_anim_state = SPIDER_WALK1;
            else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
                item->goal_anim_state = SPIDER_WALK2;
            break;

        case SPIDER_WALK1:
            if (spider->mood == BORED_MOOD)
                break;
            else if (info.ahead && item->touch_bits)
                item->goal_anim_state = SPIDER_STOP;
            else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
                item->goal_anim_state = SPIDER_WALK2;
            break;

        case SPIDER_WALK2:
            spider->flags = 0;
            if (info.ahead && item->touch_bits)
                item->goal_anim_state = SPIDER_STOP;
            else if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
                item->goal_anim_state = SPIDER_WALK1;
            break;

        case SPIDER_ATTACK1:
            if (!spider->flags && item->touch_bits)
            {
                CreatureEffect(item, &spider_bite, DoBloodSplat);
                lara_item->hit_points -= BIG_SPIDER_DAMAGE;
                lara_item->hit_status = 1;
                spider->flags = 1;
            }
            break;
        }
    }

    CreatureAnimation(item_number, angle, 0);
}
