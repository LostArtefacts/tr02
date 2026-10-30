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

#define YETI 45
#define EXTRA_YETIKILL 2
#define YETI_PUNCH_DAMAGE 100
#define YETI_THUMP_DAMAGE 150
#define YETI_CHARGE_DAMAGE 200
#define YETI_DIE_ANIM 31
#define YETI_CLIMB1_ANIM 34
#define YETI_CLIMB2_ANIM 33
#define YETI_CLIMB3_ANIM 32
#define YETI_FALL3_ANIM 35
#define YETI_KILL_ANIM 36
#define YETI_WALK_TURN (ONE_DEGREE * 4)
#define YETI_RUN_TURN (ONE_DEGREE * 6)
#define YETI_ATTACK1_RANGE SQUARE(WALL_L / 2)
#define YETI_ATTACK2_RANGE SQUARE(WALL_L * 2 / 3)
#define YETI_ATTACK3_RANGE SQUARE(WALL_L * 2)
#define YETI_RUN_RANGE SQUARE(WALL_L * 2)
#define YETI_CLOSE_RANGE YETI_ATTACK2_RANGE
#define YETI_WAIT1_CHANCE 0x100
#define YETI_WAIT2_CHANCE (YETI_WAIT1_CHANCE + 0x100)
#define YETI_WALK_CHANCE (YETI_WAIT2_CHANCE + 0x100)
#define YETI_STOP_ROAR_CHANCE 0x200
#define YETI_TOUCHL 0x3800
#define YETI_TOUCHR 0x0700
#define YETI_VAULT_SHIFT 300

enum yeti_anims {
    YETI_EMPTY,
    YETI_RUN,
    YETI_STOP,
    YETI_WALK,
    YETI_ATTACK1,
    YETI_ATTACK2,
    YETI_ATTACK3,
    YETI_WAIT1,
    YETI_DEATH,
    YETI_WAIT2,
    YETI_CLIMB1,
    YETI_CLIMB2,
    YETI_CLIMB3,
    YETI_FALL3,
    YETI_KILL
};

extern BITE_INFO yeti_biteL, yeti_biteR;

void CreatureNeck(ITEM_INFO *item, sint16 required);
int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

void YetiControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *yeti;
    sint16 angle, head, body, random;
    AI_INFO info;
    int lara_alive;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    yeti = (CREATURE_INFO *)item->data;
    angle = head = body = 0;
    lara_alive = (lara_item->hit_points > 0);

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != YETI_DEATH)
        {
            item->anim_number = objects[YETI].anim_index + YETI_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = YETI_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);

        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, yeti->maximum_turn);

        switch (item->current_anim_state)
        {
        case YETI_STOP:
            yeti->flags = 0;
            yeti->maximum_turn = 0;
            if (yeti->mood == ESCAPE_MOOD)
                item->goal_anim_state = YETI_RUN;
            else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (yeti->mood == BORED_MOOD)
            {
                random = (sint16)GetRandomControl();
                if (random < YETI_WAIT1_CHANCE || !lara_alive)
                    item->goal_anim_state = YETI_WAIT1;
                else if (random < YETI_WAIT2_CHANCE)
                    item->goal_anim_state = YETI_WAIT2;
                else if (random < YETI_WALK_CHANCE)
                    item->goal_anim_state = YETI_WALK;
            }
            else if (info.ahead && info.distance < YETI_ATTACK1_RANGE && GetRandomControl() < 0x4000)
                item->goal_anim_state = YETI_ATTACK1;
            else if (info.ahead && info.distance < YETI_ATTACK2_RANGE)
                item->goal_anim_state = YETI_ATTACK2;
            else if (yeti->mood == STALK_MOOD)
                item->goal_anim_state = YETI_WALK;
            else
                item->goal_anim_state = YETI_RUN;
            break;

        case YETI_WAIT1:
            if (yeti->mood == ESCAPE_MOOD || item->hit_status)
                item->goal_anim_state = YETI_STOP;
            else if (yeti->mood == BORED_MOOD)
            {
                if (lara_alive)
                {
                    random = (sint16)GetRandomControl();
                    if (random < YETI_WAIT1_CHANCE)
                        item->goal_anim_state = YETI_STOP;
                    else if (random < YETI_WAIT2_CHANCE)
                        item->goal_anim_state = YETI_WAIT2;
                    else if (random < YETI_WALK_CHANCE)
                    {
                        item->goal_anim_state = YETI_STOP;
                        item->required_anim_state = YETI_WALK;
                    }
                }
            }
            else if (GetRandomControl() < YETI_STOP_ROAR_CHANCE)
                item->goal_anim_state = YETI_STOP;
            break;

        case YETI_WAIT2:
            if (yeti->mood == ESCAPE_MOOD || item->hit_status)
                item->goal_anim_state = YETI_STOP;
            else if (yeti->mood == BORED_MOOD)
            {
                random = (sint16)GetRandomControl();
                if (random < YETI_WAIT1_CHANCE || !lara_alive)
                    item->goal_anim_state = YETI_WAIT1;
                else if (random < YETI_WAIT2_CHANCE)
                    item->goal_anim_state = YETI_STOP;
                else if (random < YETI_WALK_CHANCE)
                {
                    item->goal_anim_state = YETI_STOP;
                    item->required_anim_state = YETI_WALK;
                }
            }
            else if (GetRandomControl() < YETI_STOP_ROAR_CHANCE)
                item->goal_anim_state = YETI_STOP;
            break;

        case YETI_WALK:
            yeti->maximum_turn = YETI_WALK_TURN;
            if (yeti->mood == ESCAPE_MOOD)
                item->goal_anim_state = YETI_RUN;
            else if (yeti->mood == BORED_MOOD)
            {
                random = (sint16)GetRandomControl();
                if (random < YETI_WAIT1_CHANCE || !lara_alive)
                {
                    item->goal_anim_state = YETI_STOP;
                    item->required_anim_state = YETI_WAIT1;
                }
                else if (random < YETI_WAIT2_CHANCE)
                {
                    item->goal_anim_state = YETI_STOP;
                    item->required_anim_state = YETI_WAIT2;
                }
                else if (random < YETI_WALK_CHANCE)
                    item->goal_anim_state = YETI_STOP;
            }
            else if (yeti->mood == ATTACK_MOOD)
            {
                if (info.ahead && info.distance < YETI_CLOSE_RANGE)
                    item->goal_anim_state = YETI_STOP;
                else if (info.distance > YETI_RUN_RANGE)
                    item->goal_anim_state = YETI_RUN;
            }
            break;

        case YETI_RUN:
            yeti->flags = 0;
            yeti->maximum_turn = YETI_RUN_TURN;
            if (yeti->mood == ESCAPE_MOOD)
                break;
            else if (yeti->mood == BORED_MOOD)
                item->goal_anim_state = YETI_WALK;
            else if (info.ahead && info.distance < YETI_ATTACK2_RANGE)
                item->goal_anim_state = YETI_STOP;
            else if (info.ahead && info.distance < YETI_ATTACK3_RANGE)
                item->goal_anim_state = YETI_ATTACK3;
            else if (yeti->mood == STALK_MOOD)
                item->goal_anim_state = YETI_WALK;
            break;

        case YETI_ATTACK1:
            body = head;
            head = 0;
            if (!yeti->flags && (item->touch_bits & YETI_TOUCHR))
            {
                CreatureEffect(item, &yeti_biteR, DoBloodSplat);
                lara_item->hit_points -= YETI_PUNCH_DAMAGE;
                lara_item->hit_status = 1;
                yeti->flags = 1;
            }
            break;

        case YETI_ATTACK2:
            body = head;
            head = 0;
            yeti->maximum_turn = YETI_WALK_TURN;
            if (!yeti->flags && (item->touch_bits & (YETI_TOUCHR | YETI_TOUCHL)))
            {
                if (item->touch_bits & YETI_TOUCHL)
                    CreatureEffect(item, &yeti_biteL, DoBloodSplat);
                if (item->touch_bits & YETI_TOUCHR)
                    CreatureEffect(item, &yeti_biteR, DoBloodSplat);
                lara_item->hit_points -= YETI_THUMP_DAMAGE;
                lara_item->hit_status = 1;
                yeti->flags = 1;
            }
            break;

        case YETI_ATTACK3:
            body = head;
            head = 0;
            if (!yeti->flags && (item->touch_bits & (YETI_TOUCHR | YETI_TOUCHL)))
            {
                if (item->touch_bits & YETI_TOUCHL)
                    CreatureEffect(item, &yeti_biteL, DoBloodSplat);
                if (item->touch_bits & YETI_TOUCHR)
                    CreatureEffect(item, &yeti_biteR, DoBloodSplat);
                lara_item->hit_points -= YETI_CHARGE_DAMAGE;
                lara_item->hit_status = 1;
                yeti->flags = 1;
            }
            break;
        }
    }

    if (lara_alive && lara_item->hit_points <= 0)
    {
        CreatureKill(item, YETI_KILL_ANIM, YETI_KILL, EXTRA_YETIKILL);
        return;
    }

    CreatureHead(item, body);
    CreatureNeck(item, head);

    if (item->current_anim_state < YETI_CLIMB1)
    {
        switch (CreatureVault(item_number, angle, 2, YETI_VAULT_SHIFT))
        {
        case 2:
            item->anim_number = objects[YETI].anim_index + YETI_CLIMB1_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = YETI_CLIMB1;
            break;

        case 3:
            item->anim_number = objects[YETI].anim_index + YETI_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = YETI_CLIMB2;
            break;

        case 4:
            item->anim_number = objects[YETI].anim_index + YETI_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = YETI_CLIMB3;
            break;

        case -4:
            item->anim_number = objects[YETI].anim_index + YETI_FALL3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = YETI_FALL3;
            break;
        }
    }
    else
    {
        CreatureAnimation(item_number, angle, 0);
    }
}
