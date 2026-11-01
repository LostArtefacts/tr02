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

#define SMALL_RAT 21
#define MOUSE_RUN 1
#define MOUSE_STOP 2
#define MOUSE_WAIT1 3
#define MOUSE_WAIT2 4
#define MOUSE_ATTACK 5
#define MOUSE_DEATH 6
#define MOUSE_DIE_ANIM 9
#define MOUSE_DAMAGE 20
#define MOUSE_TOUCH 0x7f
#define MOUSE_RUN_TURN (6 * ONE_DEGREE)
#define MOUSE_ATTACK_RANGE SQUARE(WALL_L / 3)
#define MOUSE_WAIT1_CHANCE 0x500
#define MOUSE_WAIT2_CHANCE 0xa00

extern BITE_INFO mouse_bite;

void MouseControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *mouse;
    sint16 head, angle, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    mouse = (CREATURE_INFO *)item->data;
    head = angle = 0;

    if (item->hit_points <= 0) {
        if (item->current_anim_state != MOUSE_DEATH) {
            item->anim_number = objects[SMALL_RAT].anim_index + MOUSE_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MOUSE_DEATH;
        }
    } else {
        CreatureAIInfo(item, &info);

        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 0);

        angle = CreatureTurn(item, MOUSE_RUN_TURN);

        switch (item->current_anim_state) {
        case MOUSE_WAIT2:
            if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD) {
                random = (sint16)GetRandomControl();
                if (random < MOUSE_WAIT1_CHANCE)
                    item->required_anim_state = MOUSE_WAIT1;
                else if (random > MOUSE_WAIT2_CHANCE)
                    item->required_anim_state = MOUSE_RUN;
            } else if (info.distance < MOUSE_ATTACK_RANGE) {
                item->required_anim_state = MOUSE_ATTACK;
            } else {
                item->required_anim_state = MOUSE_RUN;
            }

            if (item->required_anim_state)
                item->goal_anim_state = MOUSE_STOP;
            break;

        case MOUSE_STOP:
            mouse->maximum_turn = 0;

            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            break;

        case MOUSE_RUN:
            mouse->maximum_turn = MOUSE_RUN_TURN;

            if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD) {
                random = (sint16)GetRandomControl();
                if (random < MOUSE_WAIT1_CHANCE) {
                    item->required_anim_state = MOUSE_WAIT1;
                    item->goal_anim_state = MOUSE_STOP;
                } else if (random < MOUSE_WAIT2_CHANCE) {
                    item->goal_anim_state = MOUSE_STOP;
                }
            } else if (info.ahead && info.distance < MOUSE_ATTACK_RANGE) {
                item->goal_anim_state = MOUSE_STOP;
            }
            break;

        case MOUSE_ATTACK:
            if (!item->required_anim_state && (item->touch_bits & MOUSE_TOUCH)) {
                CreatureEffect(item, &mouse_bite, DoBloodSplat);
                lara_item->hit_points -= MOUSE_DAMAGE;
                lara_item->hit_status = 1;
                item->required_anim_state = MOUSE_STOP;
            }
            break;

        case MOUSE_WAIT1:
            if (GetRandomControl() < MOUSE_WAIT1_CHANCE)
                item->goal_anim_state = MOUSE_STOP;
            break;
        }
    }

    CreatureHead(item, head);

    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR2_EXE
BITE_INFO mouse_bite = { 0, 0, 57, 2 };
#endif
