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

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
void DisableBaddieAI(sint16 item_number);

#define JELLY_MOVE 1
#define JELLY_STOP 2
#define JELLY_TURN (ONE_DEGREE * 90)
#define JELLY_STING 5

void JellyControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *jelly;
    sint16 angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    jelly = (CREATURE_INFO *)items[item_number].data;
    item = &items[item_number];

    if (item->hit_points <= 0)
    {
        if (ExplodingDeath(item_number, -1, 0))
        {
            DisableBaddieAI(item_number);
            KillItem(item_number);
            item->status = DEACTIVATED;
        }
        return;
    }

    CreatureAIInfo(&items[item_number], &info);
    CreatureMood(item, &info, 0);
    angle = CreatureTurn(item, JELLY_TURN);

    switch (item->current_anim_state)
    {
    case JELLY_STOP:
        if (jelly->mood != BORED_MOOD)
            item->goal_anim_state = JELLY_MOVE;
        break;

    case JELLY_MOVE:
        if (jelly->mood == BORED_MOOD || item->touch_bits)
            item->goal_anim_state = JELLY_STOP;
        break;
    }

    if (item->touch_bits)
    {
        lara_item->hit_status = 1;
        lara_item->hit_points -= JELLY_STING;
    }

    CreatureHead(item, 0);
    CreatureAnimation(item_number, angle, 0);
    CreatureUnderwater(item, 0);
}

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

void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

#define SHARK 25
#define SHARK_BITE 400
#define SHARK_STOP 0
#define SHARK_SWIM1 1
#define SHARK_SWIM2 2
#define SHARK_ATTACK1 3
#define SHARK_ATTACK2 4
#define SHARK_DEATH 5
#define SHARK_KILL 6
#define SHARK_SWIM2_RANGE SQUARE(WALL_L * 3)
#define SHARK_ATTACK1_RANGE SQUARE(WALL_L * 3 / 4)
#define SHARK_ATTACK2_RANGE SQUARE(WALL_L * 4 / 3)
#define SHARK_SWIM1_TURN (ONE_DEGREE / 2)
#define SHARK_SWIM2_TURN (ONE_DEGREE * 2)
#define SHARK_ATTACK1_CHANCE 0x800
#define SHARK_DIE_ANIM 4
#define SHARK_KILL_ANIM 19
#define SHARK_TOUCH 0x3400
#define EXTRA_SHARKKILL 3

extern BITE_INFO shark_bite;

void SharkControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *shark;
    sint16 angle, head;
    AI_INFO info;
    int lara_alive;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    shark = (CREATURE_INFO *)item->data;
    angle = head = 0;
    lara_alive = (lara_item->hit_points > 0);

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != SHARK_DEATH)
        {
            item->anim_number = objects[SHARK].anim_index + SHARK_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = SHARK_DEATH;
        }
        CreatureFloat(item_number);
        return;
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, shark->maximum_turn);

        switch (item->current_anim_state)
        {
        case SHARK_STOP:
            shark->flags = 0;
            shark->maximum_turn = 0;
            if (info.ahead && info.distance < SHARK_ATTACK1_RANGE && info.zone_number == info.enemy_zone)
                item->goal_anim_state = SHARK_ATTACK1;
            else
                item->goal_anim_state = SHARK_SWIM1;
            break;

        case SHARK_SWIM1:
            shark->maximum_turn = SHARK_SWIM1_TURN;
            if (shark->mood == BORED_MOOD)
                break;
            else if (info.ahead && info.distance < SHARK_ATTACK1_RANGE)
                item->goal_anim_state = SHARK_STOP;
            else if (shark->mood == ESCAPE_MOOD || info.distance > SHARK_SWIM2_RANGE || !info.ahead)
                item->goal_anim_state = SHARK_SWIM2;
            break;

        case SHARK_SWIM2:
            shark->flags = 0;
            shark->maximum_turn = SHARK_SWIM2_TURN;
            if (shark->mood == BORED_MOOD)
                item->goal_anim_state = SHARK_SWIM1;
            else if (shark->mood == ESCAPE_MOOD)
                break;
            else if (info.ahead && info.distance < SHARK_ATTACK2_RANGE && info.zone_number == info.enemy_zone)
            {
                if (GetRandomControl() < SHARK_ATTACK1_CHANCE)
                    item->goal_anim_state = SHARK_STOP;
                else if (info.distance < SHARK_ATTACK1_RANGE)
                    item->goal_anim_state = SHARK_ATTACK2;
            }
            break;

        case SHARK_ATTACK1:
        case SHARK_ATTACK2:
            if (info.ahead)
                head = info.angle;
            if (!shark->flags && (item->touch_bits & SHARK_TOUCH))
            {
                lara_item->hit_points -= SHARK_BITE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &shark_bite, DoBloodSplat);
                shark->flags = 1;
            }
            break;
        }
    }

    if (lara_alive && lara_item->hit_points <= 0)
    {
        CreatureKill(item, SHARK_KILL_ANIM, SHARK_KILL, EXTRA_SHARKKILL);
        return;
    }

    if (item->current_anim_state != SHARK_KILL)
    {
        CreatureHead(item, head);
        CreatureAnimation(item_number, angle, 0);
        CreatureUnderwater(item, WALL_L / 3);
    }
    else
        AnimateItem(item);
}
