#include "game.h"
#include "box.h"

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
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

#define SQUARE(A) ((A) * (A))

#define BIG_YETI 46
#define BIRDY_PUNCH_DAMAGE 200
#define BIRDY_EMPTY 0
#define BIRDY_WAIT 1
#define BIRDY_WALK 2
#define BIRDY_AIM1 3
#define BIRDY_PUNCH1 4
#define BIRDY_AIM2 5
#define BIRDY_PUNCH2 6
#define BIRDY_PUNCHR 7
#define BIRDY_WAIT2 8
#define BIRDY_DEATH 9
#define BIRDY_AIM3 10
#define BIRDY_PUNCH3 11
#define BIRDY_DIE_ANIM 20
#define BIRDY_WALK_TURN (ONE_DEGREE * 4)
#define BIRDY_ATTACK1_RANGE SQUARE(WALL_L)
#define BIRDY_ATTACK2_RANGE SQUARE(WALL_L * 2)
#define BIRDY_TOUCHL 0x0c0000
#define BIRDY_TOUCHR 0x600000

extern BITE_INFO birdy_left, birdy_right;

void GiantYetiControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *birdy;
    AI_INFO info;
    sint16 angle, head, body;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    birdy = (CREATURE_INFO *)item->data;
    angle = head = body = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != BIRDY_DEATH)
        {
            item->anim_number = objects[BIG_YETI].anim_index + BIRDY_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BIRDY_DEATH;
        }
        if (item->frame_number == anims[item->anim_number].frame_end)
            level_complete = 1;
    }
    else
    {
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, birdy->maximum_turn);

        switch (item->current_anim_state)
        {
        case BIRDY_WAIT:
            birdy->maximum_turn = 0;
            if (info.ahead && info.distance < BIRDY_ATTACK1_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = BIRDY_AIM1;
                else
                    item->goal_anim_state = BIRDY_AIM3;
            }
            else if ((birdy->mood == BORED_MOOD || birdy->mood == STALK_MOOD) && info.ahead)
                item->goal_anim_state = BIRDY_WAIT2;
            else
                item->goal_anim_state = BIRDY_WALK;
            break;

        case BIRDY_WAIT2:
            if (birdy->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = BIRDY_WAIT;
            break;

        case BIRDY_WALK:
            birdy->maximum_turn = BIRDY_WALK_TURN;
            if (info.ahead && info.distance < BIRDY_ATTACK2_RANGE)
                item->goal_anim_state = BIRDY_AIM2;
            else if ((birdy->mood == BORED_MOOD || birdy->mood == STALK_MOOD) && info.ahead)
                item->goal_anim_state = BIRDY_WAIT;
            break;

        case BIRDY_AIM1:
            birdy->flags = 0;
            if (info.ahead && info.distance < BIRDY_ATTACK1_RANGE)
                item->goal_anim_state = BIRDY_PUNCH1;
            else
                item->goal_anim_state = BIRDY_WAIT;
            break;

        case BIRDY_AIM2:
            birdy->flags = 0;
            if (info.ahead && info.distance < BIRDY_ATTACK2_RANGE)
                item->goal_anim_state = BIRDY_PUNCH2;
            else
                item->goal_anim_state = BIRDY_WALK;
            break;

        case BIRDY_AIM3:
            birdy->flags = 0;
            if (info.ahead && info.distance < BIRDY_ATTACK1_RANGE)
                item->goal_anim_state = BIRDY_PUNCH3;
            else
                item->goal_anim_state = BIRDY_WAIT;
            break;

        case BIRDY_PUNCH1:
        case BIRDY_PUNCH2:
        case BIRDY_PUNCHR:
        case BIRDY_PUNCH3:
            if (!(birdy->flags & 1) && (item->touch_bits & BIRDY_TOUCHR))
            {
                CreatureEffect(item, &birdy_right, DoBloodSplat);
                lara_item->hit_points -= BIRDY_PUNCH_DAMAGE;
                lara_item->hit_status = 1;
                birdy->flags |= 1;
            }
            if (!(birdy->flags & 2) && (item->touch_bits & BIRDY_TOUCHL))
            {
                CreatureEffect(item, &birdy_left, DoBloodSplat);
                lara_item->hit_points -= BIRDY_PUNCH_DAMAGE;
                lara_item->hit_status = 1;
                birdy->flags |= 2;
            }
            break;
        }
    }

    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, 0);
}
