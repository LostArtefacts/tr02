#include "game.h"
#include "box.h"

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

void InitialiseCreature(sint16 item_number);
int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE direction, sint16 room_num);

extern BITE_INFO bird_bite;
extern BITE_INFO crow_bite;

#define SQUARE(A) ((A) * (A))

#define CROW 38
#define EAGLE 47

#define BIRD_EMPTY 0
#define BIRD_FLY 1
#define BIRD_STOP 2
#define BIRD_GLIDE 3
#define BIRD_FALL 4
#define BIRD_DEATH 5
#define BIRD_ATTACK 6
#define BIRD_EAT 7

#define BIRD_DAMAGE 20
#define BIRD_TURN (ONE_DEGREE * 3)
#define BIRD_ATTACK_RANGE SQUARE(WALL_L / 2)
#define BIRD_DIE_ANIM 8
#define CROW_DIE_ANIM 1
#define BIRD_START_ANIM 5
#define CROW_START_ANIM 14

void InitialiseEagle(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);

    item = &items[item_number];
    if (item->object_number == CROW) {
        item->anim_number = objects[CROW].anim_index + CROW_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_EAT;
    } else {
        item->anim_number = objects[EAGLE].anim_index + BIRD_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_STOP;
    }
}

void EagleControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *bird;
    AI_INFO info;
    sint16 angle;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    bird = (CREATURE_INFO *)item->data;

    if (item->hit_points <= 0) {
        switch (item->current_anim_state) {
        case BIRD_FALL:
            if (item->pos.y_pos > item->floor) {
                item->pos.y_pos = item->floor;
                item->gravity_status = 0;
                item->fallspeed = 0;
                item->goal_anim_state = BIRD_DEATH;
            }
            break;

        case BIRD_DEATH:
            item->pos.y_pos = item->floor;
            break;

        default:
            if (item->object_number == CROW)
                item->anim_number = objects[CROW].anim_index + CROW_DIE_ANIM;
            else
                item->anim_number = objects[EAGLE].anim_index + BIRD_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BIRD_FALL;
            item->gravity_status = 1;
            item->speed = 0;
            break;
        }
        item->pos.x_rot = 0;
    } else {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, BIRD_TURN);

        switch (item->current_anim_state) {
        case BIRD_EAT:
            item->pos.y_pos = item->floor;
            if (bird->mood != BORED_MOOD)
                item->goal_anim_state = BIRD_FLY;
            break;

        case BIRD_STOP:
            item->pos.y_pos = item->floor;
            if (bird->mood != BORED_MOOD)
                item->goal_anim_state = BIRD_FLY;
            break;

        case BIRD_FLY:
            bird->flags = 0;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            if (bird->mood == BORED_MOOD)
                item->goal_anim_state = BIRD_STOP;
            else if (info.ahead && info.distance < BIRD_ATTACK_RANGE)
                item->goal_anim_state = BIRD_ATTACK;
            else
                item->goal_anim_state = BIRD_GLIDE;
            break;

        case BIRD_GLIDE:
            if (bird->mood == BORED_MOOD) {
                item->required_anim_state = BIRD_STOP;
                item->goal_anim_state = BIRD_FLY;
            } else if (info.ahead && info.distance < BIRD_ATTACK_RANGE)
                item->goal_anim_state = BIRD_ATTACK;
            break;

        case BIRD_ATTACK:
            if (!bird->flags && item->touch_bits) {
                lara_item->hit_points -= BIRD_DAMAGE;
                lara_item->hit_status = 1;
                if (item->object_number == CROW)
                    CreatureEffect(item, &crow_bite, DoBloodSplat);
                else
                    CreatureEffect(item, &bird_bite, DoBloodSplat);
                bird->flags = 1;
            }
            break;
        }
    }

    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR2_EXE
BITE_INFO bird_bite = { 15, 46, 21, 6 };
BITE_INFO crow_bite = { 2, 10, 60, 14 };
#endif
