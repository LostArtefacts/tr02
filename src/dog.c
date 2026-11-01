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

#define DOG 15
#define DOG_BITE_DAMAGE 100
#define DOG_LEAP_DAMAGE 200
#define DOG_LUNGE_DAMAGE 100
#define DOG_TOUCH 0x13f7000
#define DOG_EMPTY 0
#define DOG_WALK 1
#define DOG_RUN 2
#define DOG_STOP 3
#define DOG_BARK 4
#define DOG_CROUCH 5
#define DOG_STAND 6
#define DOG_ATTACK1 7
#define DOG_ATTACK2 8
#define DOG_ATTACK3 9
#define DOG_DEATH 10
#define DOG_DIE_ANIM 13
#define DOG_WALK_TURN (3 * ONE_DEGREE)
#define DOG_RUN_TURN (6 * ONE_DEGREE)
#define DOG_ATTACK1_RANGE SQUARE(WALL_L / 3)
#define DOG_ATTACK2_RANGE SQUARE(WALL_L * 3 / 4)
#define DOG_ATTACK3_RANGE SQUARE(WALL_L * 2 / 3)
#define DOG_BARK_CHANCE 0x300
#define DOG_CROUCH_CHANCE (DOG_BARK_CHANCE + 0x300)
#define DOG_STAND_CHANCE (DOG_CROUCH_CHANCE + 0x500)
#define DOG_WALK_CHANCE (DOG_CROUCH_CHANCE + 0x2000)
#define DOG_UNCROUCH_CHANCE 0x100
#define DOG_UNSTAND_CHANCE 0x200
#define DOG_UNBARK_CHANCE 0x500

extern BITE_INFO dog_bite;

void DogControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *dog;
    sint16 angle, head, tilt, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    dog = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != DOG_DEATH)
        {
            item->anim_number = objects[DOG].anim_index + DOG_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = DOG_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, dog->maximum_turn);

        switch (item->current_anim_state)
        {
        case DOG_STOP:
            dog->maximum_turn = 0;
            dog->flags = 0;
            if (dog->mood == BORED_MOOD)
            {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else
                {
                    random = (sint16)GetRandomControl();
                    if (random < DOG_BARK_CHANCE)
                        item->goal_anim_state = DOG_BARK;
                    else if (random < DOG_CROUCH_CHANCE)
                        item->goal_anim_state = DOG_CROUCH;
                    else if (random < DOG_WALK_CHANCE)
                        item->goal_anim_state = DOG_WALK;
                }
            }
            else if (dog->mood == ESCAPE_MOOD)
                item->goal_anim_state = DOG_RUN;
            else if (info.distance < DOG_ATTACK1_RANGE && info.ahead)
                item->goal_anim_state = DOG_ATTACK1;
            else
                item->goal_anim_state = DOG_RUN;
            break;

        case DOG_WALK:
            dog->maximum_turn = DOG_WALK_TURN;
            if (dog->mood == BORED_MOOD)
            {
                random = (sint16)GetRandomControl();
                if (random < DOG_BARK_CHANCE)
                {
                    item->required_anim_state = DOG_BARK;
                    item->goal_anim_state = DOG_STOP;
                }
                else if (random < DOG_CROUCH_CHANCE)
                {
                    item->required_anim_state = DOG_CROUCH;
                    item->goal_anim_state = DOG_STOP;
                }
                else if (random < DOG_STAND_CHANCE)
                    item->goal_anim_state = DOG_STOP;
            }
            else
                item->goal_anim_state = DOG_RUN;
            break;

        case DOG_RUN:
            tilt = angle;
            dog->maximum_turn = DOG_RUN_TURN;
            if (dog->mood == BORED_MOOD)
                item->goal_anim_state = DOG_STOP;
            else if (info.distance < DOG_ATTACK2_RANGE)
                item->goal_anim_state = DOG_ATTACK2;
            break;

        case DOG_ATTACK2:
            if (dog->flags != 2 && (item->touch_bits & DOG_TOUCH))
            {
                CreatureEffect(item, &dog_bite, DoBloodSplat);
                lara_item->hit_points -= DOG_LEAP_DAMAGE;
                lara_item->hit_status = 1;
                dog->flags = 2;
            }
            if (info.distance < DOG_ATTACK1_RANGE)
                item->goal_anim_state = DOG_ATTACK1;
            else if (info.distance < DOG_ATTACK3_RANGE)
                item->goal_anim_state = DOG_ATTACK3;
            break;

        case DOG_ATTACK1:
            dog->maximum_turn = 0;
            if (dog->flags != 1 && info.ahead && (item->touch_bits & DOG_TOUCH))
            {
                CreatureEffect(item, &dog_bite, DoBloodSplat);
                lara_item->hit_points -= DOG_BITE_DAMAGE;
                lara_item->hit_status = 1;
                dog->flags = 1;
            }
            if (info.distance > DOG_ATTACK1_RANGE && info.distance < DOG_ATTACK3_RANGE)
                item->goal_anim_state = DOG_ATTACK3;
            else
                item->goal_anim_state = DOG_STOP;
            break;

        case DOG_ATTACK3:
            dog->maximum_turn = DOG_RUN_TURN;
            if (dog->flags != 3 && (item->touch_bits & DOG_TOUCH))
            {
                CreatureEffect(item, &dog_bite, DoBloodSplat);
                lara_item->hit_points -= DOG_LUNGE_DAMAGE;
                lara_item->hit_status = 1;
                dog->flags = 3;
            }
            if (info.distance < DOG_ATTACK1_RANGE)
                item->goal_anim_state = DOG_ATTACK1;
            break;

        case DOG_BARK:
            if (dog->mood != BORED_MOOD || GetRandomControl() < DOG_UNBARK_CHANCE)
                item->goal_anim_state = DOG_STOP;
            break;

        case DOG_CROUCH:
            if (dog->mood != BORED_MOOD || GetRandomControl() < DOG_UNCROUCH_CHANCE)
                item->goal_anim_state = DOG_STOP;
            break;

        case DOG_STAND:
            if (dog->mood != BORED_MOOD || GetRandomControl() < DOG_UNSTAND_CHANCE)
                item->goal_anim_state = DOG_STOP;
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, tilt);
}

#define TIGER 39
#define TIGER_BITE_DAMAGE 100
#define TIGER_TOUCH 0x7fdc000
#define TIGER_EMPTY 0
#define TIGER_STOP 1
#define TIGER_WALK 2
#define TIGER_RUN 3
#define TIGER_WAIT 4
#define TIGER_ROAR 5
#define TIGER_ATTACK1 6
#define TIGER_ATTACK2 7
#define TIGER_ATTACK3 8
#define TIGER_DEATH 9
#define TIGER_DIE_ANIM 11
#define TIGER_WALK_TURN (3 * ONE_DEGREE)
#define TIGER_RUN_TURN (6 * ONE_DEGREE)
#define TIGER_ATTACK1_RANGE SQUARE(WALL_L / 3)
#define TIGER_ATTACK2_RANGE SQUARE(WALL_L * 3 / 2)
#define TIGER_ATTACK3_RANGE SQUARE(WALL_L)
#define TIGER_ROAR_CHANCE 0x60
#define TIGER_WALK_CHANCE (TIGER_ROAR_CHANCE + 0x400)

extern BITE_INFO tiger_bite;

void TigerControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *tiger;
    sint16 angle, head, tilt, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    tiger = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != TIGER_DEATH)
        {
            item->anim_number = objects[TIGER].anim_index + TIGER_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = TIGER_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, tiger->maximum_turn);

        switch (item->current_anim_state)
        {
        case TIGER_STOP:
            tiger->maximum_turn = 0;
            tiger->flags = 0;
            if (tiger->mood == ESCAPE_MOOD)
                item->goal_anim_state = TIGER_RUN;
            else if (tiger->mood == BORED_MOOD)
            {
                random = (sint16)GetRandomControl();
                if (random < TIGER_ROAR_CHANCE)
                    item->goal_anim_state = TIGER_ROAR;
                else if (random < TIGER_WALK_CHANCE);
                    item->goal_anim_state = TIGER_WALK;
            }
            else if (info.ahead && info.distance < TIGER_ATTACK1_RANGE)
                item->goal_anim_state = TIGER_ATTACK1;
            else if (info.ahead && info.distance < TIGER_ATTACK3_RANGE)
            {
                tiger->maximum_turn = TIGER_WALK_TURN;
                item->goal_anim_state = TIGER_ATTACK3;
            }
            else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < TIGER_ROAR_CHANCE)
                item->goal_anim_state = TIGER_ROAR;
            else
                item->goal_anim_state = TIGER_RUN;
            break;

        case TIGER_WALK:
            tiger->maximum_turn = TIGER_WALK_TURN;
            if (GetRandomControl() < TIGER_ROAR_CHANCE)
            {
                item->goal_anim_state = TIGER_STOP;
                item->required_anim_state = TIGER_ROAR;
            }
            break;

        case TIGER_RUN:
            tiger->maximum_turn = TIGER_RUN_TURN;
            if (tiger->mood == BORED_MOOD)
                item->goal_anim_state = TIGER_STOP;
            else if (tiger->flags && info.ahead)
                item->goal_anim_state = TIGER_STOP;
            else if (info.ahead && info.distance < TIGER_ATTACK2_RANGE)
            {
                if (lara_item->speed == 0)
                    item->goal_anim_state = TIGER_STOP;
                else
                    item->goal_anim_state = TIGER_ATTACK2;
            }
            else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < TIGER_ROAR_CHANCE)
            {
                item->required_anim_state = TIGER_ROAR;
                item->goal_anim_state = TIGER_STOP;
            }
            tiger->flags = 0;
            break;

        case TIGER_ATTACK1:
        case TIGER_ATTACK2:
        case TIGER_ATTACK3:
            if (!tiger->flags && (item->touch_bits & TIGER_TOUCH))
            {
                lara_item->hit_status = 1;
                lara_item->hit_points -= TIGER_BITE_DAMAGE;
                CreatureEffect(item, &tiger_bite, DoBloodSplat);
                tiger->flags = 1;
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, tilt);
}

#ifdef TR2_EXE
BITE_INFO dog_bite = { 0, 30, 141, 20 };
BITE_INFO tiger_bite = { 19, -13, 3, 26 };
#endif
