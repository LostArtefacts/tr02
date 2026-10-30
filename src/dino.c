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
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

#define SQUARE(A) ((A) * (A))
#define FRONT_ARC 0x4000

#define DINO_TOUCH_DAMAGE 1
#define DINO_TRAMPLE_DAMAGE 10
#define DINO_BITE_DAMAGE 10000
#define DINO_TOUCH 0x3000
#define DINO_LINK 0
#define DINO_STOP 1
#define DINO_WALK 2
#define DINO_RUN 3
#define DINO_ATTACK1 4
#define DINO_DEATH 5
#define DINO_ROAR 6
#define DINO_ATTACK2 7
#define DINO_KILL 8
#define DINO_KILL_ANIM 11
#define EXTRA_DINOKILL 6
#define DINO_ROAR_CHANCE 0x200
#define DINO_RUN_TURN (ONE_DEGREE * 4)
#define DINO_WALK_TURN (ONE_DEGREE * 2)
#define DINO_RUN_RANGE SQUARE(WALL_L * 5)
#define DINO_ATTACK_RANGE SQUARE(WALL_L * 4)
#define DINO_BITE_RANGE SQUARE(1500)

void DinoControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *dino;
    sint16 head, angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    dino = (CREATURE_INFO *)item->data;
    head = angle = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state == DINO_STOP)
            item->goal_anim_state = DINO_DEATH;
        else
            item->goal_anim_state = DINO_STOP;
    }
    else
    {
        CreatureAIInfo(item, &info);

        if (info.ahead)
            head = info.angle;

        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, dino->maximum_turn);

        if (item->touch_bits)
            lara_item->hit_points -= (item->current_anim_state == DINO_RUN) ? DINO_TRAMPLE_DAMAGE : DINO_TOUCH_DAMAGE;

        dino->flags = (dino->mood != ESCAPE_MOOD && !info.ahead && info.enemy_facing > -FRONT_ARC && info.enemy_facing < FRONT_ARC);
        if (!dino->flags && info.distance > DINO_BITE_RANGE && info.distance < DINO_ATTACK_RANGE && info.bite)
            dino->flags = 1;

        switch (item->current_anim_state)
        {
        case DINO_STOP:
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (info.distance < DINO_BITE_RANGE && info.bite)
                item->goal_anim_state = DINO_ATTACK2;
            else if (dino->mood == BORED_MOOD || dino->flags)
                item->goal_anim_state = DINO_WALK;
            else
                item->goal_anim_state = DINO_RUN;
            break;

        case DINO_WALK:
            dino->maximum_turn = DINO_WALK_TURN;
            if (dino->mood != BORED_MOOD && !dino->flags)
                item->goal_anim_state = DINO_STOP;
            else if (info.ahead && GetRandomControl() < DINO_ROAR_CHANCE)
            {
                item->required_anim_state = DINO_ROAR;
                item->goal_anim_state = DINO_STOP;
            }
            break;

        case DINO_RUN:
            dino->maximum_turn = DINO_RUN_TURN;
            if (info.distance < DINO_RUN_RANGE && info.bite)
                item->goal_anim_state = DINO_STOP;
            else if (dino->flags)
                item->goal_anim_state = DINO_STOP;
            else if (dino->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < DINO_ROAR_CHANCE)
            {
                item->required_anim_state = DINO_ROAR;
                item->goal_anim_state = DINO_STOP;
            }
            else if (dino->mood == BORED_MOOD)
                item->goal_anim_state = DINO_STOP;
            break;

        case DINO_ATTACK2:
            if (item->touch_bits & DINO_TOUCH)
            {
                lara_item->hit_points -= DINO_BITE_DAMAGE;
                lara_item->hit_status = 1;
                CreatureKill(item, DINO_KILL_ANIM, DINO_KILL, EXTRA_DINOKILL);
                return;
            }
            item->required_anim_state = DINO_WALK;
            break;
        }
    }

    CreatureHead(item, (sint16)(head >> 1));
    dino->neck_rotation = dino->head_rotation;
    CreatureAnimation(item_number, angle, 0);
}
