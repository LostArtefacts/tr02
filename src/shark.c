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
void CreatureFloat(sint16 item_number);
void CreatureUnderwater(ITEM_INFO *item, sint32 depth);

#define SQUARE(A) ((A) * (A))

#define BARACUDDA 28
#define BARA_BITE 100
#define BARA_EMPTY 0
#define BARA_STOP 1
#define BARA_SWIM1 2
#define BARA_SWIM2 3
#define BARA_ATTACK1 4
#define BARA_ATTACK2 5
#define BARA_DEATH 6
#define BARA_DIE_ANIM 6
#define BARA_SWIM1_TURN (ONE_DEGREE * 2)
#define BARA_SWIM2_TURN (ONE_DEGREE * 4)
#define BARA_ATTACK1_RANGE SQUARE(WALL_L * 2 / 3)
#define BARA_ATTACK2_RANGE SQUARE(WALL_L / 3)
#define BARA_TOUCH 0xe0

extern BITE_INFO bara_bite;

void BaracuddaControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *bara;
    sint16 angle, head;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    bara = (CREATURE_INFO *)item->data;
    angle = head = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != BARA_DEATH)
        {
            item->anim_number = objects[BARACUDDA].anim_index + BARA_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BARA_DEATH;
        }
        CreatureFloat(item_number);
        return;
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, bara->maximum_turn);

        switch (item->current_anim_state)
        {
        case BARA_STOP:
            bara->flags = 0;
            if (bara->mood == BORED_MOOD)
                item->goal_anim_state = BARA_SWIM1;
            else if (info.ahead && info.distance < BARA_ATTACK1_RANGE)
                item->goal_anim_state = BARA_ATTACK1;
            else if (bara->mood == STALK_MOOD)
                item->goal_anim_state = BARA_SWIM1;
            else
                item->goal_anim_state = BARA_SWIM2;
            break;

        case BARA_SWIM1:
            bara->maximum_turn = BARA_SWIM1_TURN;
            if (bara->mood == BORED_MOOD)
                break;
            else if (info.ahead && (item->touch_bits & BARA_TOUCH))
                item->goal_anim_state = BARA_STOP;
            else if (bara->mood != STALK_MOOD)
                item->goal_anim_state = BARA_SWIM2;
            break;

        case BARA_SWIM2:
            bara->maximum_turn = BARA_SWIM2_TURN;
            bara->flags = 0;
            if (bara->mood == BORED_MOOD)
                item->goal_anim_state = BARA_SWIM1;
            else if (info.ahead && info.distance < BARA_ATTACK2_RANGE)
                item->goal_anim_state = BARA_ATTACK2;
            else if (info.ahead && info.distance < BARA_ATTACK1_RANGE)
                item->goal_anim_state = BARA_STOP;
            else if (bara->mood == STALK_MOOD)
                item->goal_anim_state = BARA_SWIM1;
            break;

        case BARA_ATTACK1:
        case BARA_ATTACK2:
            if (info.ahead)
                head = info.angle;
            if (!bara->flags && (item->touch_bits & BARA_TOUCH))
            {
                lara_item->hit_points -= BARA_BITE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &bara_bite, DoBloodSplat);
                bara->flags = 1;
            }
            break;
        }
    }

    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, 0);
    CreatureUnderwater(item, STEP_L);
}
