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

#define MONK_DIE_ANIM 20
#define MONK_WALK_TURN (ONE_DEGREE * 3)
#define MONK_RUN_TURN (ONE_DEGREE * 4)
#define MONK_CLOSE_RANGE SQUARE(WALL_L / 2)
#define MONK_LONG_RANGE SQUARE(WALL_L)
#define MONK_ATTACK5_RANGE SQUARE(WALL_L * 3)
#define MONK_WALK_RANGE SQUARE(WALL_L * 2)
#define MONK_HIT_RANGE (STEP_L * 2)
#define MONK_BIFF_DAMAGE 150
#define MONK_BIFF_ENEMY_DAMAGE 5
#define MONK_TOUCH 0x4000

enum monk_anims {
    MONK_EMPTY,
    MONK_WAIT1,
    MONK_WALK,
    MONK_RUN,
    MONK_ATTACK1,
    MONK_ATTACK2,
    MONK_ATTACK3,
    MONK_ATTACK4,
    MONK_AIM3,
    MONK_DEATH,
    MONK_ATTACK5,
    MONK_WAIT2
};

extern BITE_INFO monk_hit;
extern int monks_attack_lara;

void MonkControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    CREATURE_INFO *monk;
    sint16 angle, head, tilt, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    monk = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != MONK_DEATH)
        {
            item->anim_number = objects[item->object_number].anim_index + MONK_DIE_ANIM + (GetRandomControl() / 0x4000);
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONK_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);

        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, monk->maximum_turn);

        if (info.ahead)
            head = info.angle;

        switch (item->current_anim_state)
        {
        case MONK_WAIT1:
            monk->flags &= 0x0fff;
            if (!monks_attack_lara && info.ahead && lara.target == item)
                break;
            else if (monk->mood == BORED_MOOD)
                item->goal_anim_state = MONK_WALK;
            else if (monk->mood == ESCAPE_MOOD)
                item->goal_anim_state = MONK_RUN;
            else if (info.ahead && info.distance < MONK_CLOSE_RANGE)
            {
                if (GetRandomControl() < 0x7000)
                    item->goal_anim_state = MONK_ATTACK1;
                else
                    item->goal_anim_state = MONK_WAIT2;
            }
            else if (info.ahead && info.distance < MONK_LONG_RANGE)
                item->goal_anim_state = MONK_ATTACK4;
            else if (info.ahead && info.distance < MONK_WALK_RANGE)
                item->goal_anim_state = MONK_WALK;
            else
                item->goal_anim_state = MONK_RUN;
            break;

        case MONK_WAIT2:
            monk->flags &= 0x0fff;
            if (!monks_attack_lara && info.ahead && lara.target == item)
                break;
            else if (monk->mood == BORED_MOOD)
                item->goal_anim_state = MONK_WALK;
            else if (monk->mood == ESCAPE_MOOD)
                item->goal_anim_state = MONK_RUN;
            else if (info.ahead && info.distance < MONK_CLOSE_RANGE)
            {
                random = (sint16)GetRandomControl();
                if (random < 0x3000)
                    item->goal_anim_state = MONK_ATTACK2;
                else if (random < 0x6000)
                    item->goal_anim_state = MONK_AIM3;
                else
                    item->goal_anim_state = MONK_WAIT1;
            }
            else if (info.ahead && info.distance < MONK_WALK_RANGE)
                item->goal_anim_state = MONK_WALK;
            else
                item->goal_anim_state = MONK_RUN;
            break;

        case MONK_WALK:
            monk->maximum_turn = MONK_WALK_TURN;
            if (monk->mood == BORED_MOOD)
            {
                if (!monks_attack_lara && info.ahead && lara.target == item)
                {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = MONK_WAIT1;
                    else
                        item->goal_anim_state = MONK_WAIT2;
                }
            }
            else if (monk->mood == ESCAPE_MOOD)
                item->goal_anim_state = MONK_RUN;
            else if (info.ahead && info.distance < MONK_CLOSE_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = MONK_WAIT1;
                else
                    item->goal_anim_state = MONK_WAIT2;
            }
            else if (!info.ahead || info.distance > MONK_WALK_RANGE)
                item->goal_anim_state = MONK_RUN;
            break;

        case MONK_RUN:
            monk->flags &= 0x0fff;
            monk->maximum_turn = MONK_RUN_TURN;
            if (monks_attack_lara)
                monk->maximum_turn += ONE_DEGREE;
            tilt = angle >> 2;
            if (monk->mood == BORED_MOOD)
                item->goal_anim_state = MONK_WAIT1;
            else if (monk->mood == ESCAPE_MOOD)
                break;
            else if (info.ahead && info.distance < MONK_CLOSE_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = MONK_WAIT1;
                else
                    item->goal_anim_state = MONK_WAIT2;
            }
            else if (info.ahead && info.distance < MONK_ATTACK5_RANGE)
                item->goal_anim_state = MONK_ATTACK5;
            else if (info.ahead && info.distance < MONK_WALK_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = MONK_WAIT1;
                else
                    item->goal_anim_state = MONK_WAIT2;
            }
            break;

        case MONK_AIM3:
            if (!info.ahead || info.distance > MONK_CLOSE_RANGE)
                item->goal_anim_state = MONK_WAIT2;
            else
                item->goal_anim_state = MONK_ATTACK3;
            break;

        case MONK_ATTACK1:
        case MONK_ATTACK2:
        case MONK_ATTACK3:
        case MONK_ATTACK4:
        case MONK_ATTACK5:
            enemy = monk->enemy;
            if (enemy == lara_item)
            {
                if (!(monk->flags & 0xf000) && (item->touch_bits & MONK_TOUCH))
                {
                    lara_item->hit_points -= MONK_BIFF_DAMAGE;
                    lara_item->hit_status = 1;
                    monk->flags |= 0x1000;
                    SoundEffect(245, &item->pos, 0);
                    CreatureEffect(item, &monk_hit, DoBloodSplat);
                }
            }
            else
            {
                if (!(monk->flags & 0xf000) && enemy)
                {
                    if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONK_HIT_RANGE
                        && ABS(enemy->pos.y_pos - item->pos.y_pos) < MONK_HIT_RANGE
                        && ABS(enemy->pos.z_pos - item->pos.z_pos) < MONK_HIT_RANGE)
                    {
                        enemy->hit_points -= MONK_BIFF_ENEMY_DAMAGE;
                        enemy->hit_status = 1;
                        monk->flags |= 0x1000;
                        SoundEffect(245, &item->pos, 0);
                    }
                }
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureAnimation(item_number, angle, 0);
}

#define WORKER3 32
#define WORK3_DIE_ANIM 26
#define WORK3_CLIMB1_ANIM 28
#define WORK3_CLIMB2_ANIM 29
#define WORK3_CLIMB3_ANIM 27
#define WORK3_FALL3_ANIM 30
#define WORK3_WALK_TURN (ONE_DEGREE * 5)
#define WORK3_RUN_TURN (ONE_DEGREE * 6)
#define WORK3_ATTACK0_RANGE SQUARE(WALL_L / 2)
#define WORK3_ATTACK1_RANGE SQUARE(WALL_L)
#define WORK3_ATTACK2_RANGE SQUARE(WALL_L * 3 / 2)
#define WORK3_WALK_RANGE SQUARE(WALL_L * 2)
#define WORK3_WALK_CHANCE 0x100
#define WORK3_WAIT_CHANCE 0x100
#define WORK3_HIT_DAMAGE 80
#define WORK3_SWIPE_DAMAGE 100
#define WORK3_TOUCH 0x600
#define WORK3_VAULT_SHIFT 260

enum work3_anims {
    WORK3_EMPTY,
    WORK3_STOP,
    WORK3_WALK,
    WORK3_PUNCH2,
    WORK3_AIM2,
    WORK3_WAIT,
    WORK3_AIM1,
    WORK3_AIM0,
    WORK3_PUNCH1,
    WORK3_PUNCH0,
    WORK3_RUN,
    WORK3_DEATH,
    WORK3_CLIMB3,
    WORK3_CLIMB1,
    WORK3_CLIMB2,
    WORK3_FALL3
};

extern BITE_INFO work3_hit;

int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift);

void Worker3Control(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *worker;
    sint16 angle, head, neck, tilt;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    worker = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != WORK3_DEATH)
        {
            item->anim_number = objects[WORKER3].anim_index + WORK3_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK3_DEATH;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);

        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, worker->maximum_turn);

        switch (item->current_anim_state)
        {
        case WORK3_STOP:
            worker->flags = 0;
            if (info.ahead)
                neck = info.angle;
            worker->maximum_turn = 0;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK3_RUN;
            else if (worker->mood == BORED_MOOD)
            {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = WORK3_WALK;
                else
                    item->goal_anim_state = WORK3_WAIT;
            }
            else if (info.bite && info.distance < WORK3_ATTACK0_RANGE)
                item->goal_anim_state = WORK3_AIM0;
            else if (info.bite && info.distance < WORK3_ATTACK1_RANGE)
                item->goal_anim_state = WORK3_AIM1;
            else if (info.bite && info.distance < WORK3_WALK_RANGE)
                item->goal_anim_state = WORK3_WALK;
            else
                item->goal_anim_state = WORK3_RUN;
            break;

        case WORK3_WAIT:
            if (info.ahead)
                neck = info.angle;
            if (worker->mood != BORED_MOOD)
                item->goal_anim_state = WORK3_STOP;
            else if (GetRandomControl() < WORK3_WALK_CHANCE)
            {
                item->required_anim_state = WORK3_WALK;
                item->goal_anim_state = WORK3_STOP;
            }
            break;

        case WORK3_WALK:
            if (info.ahead)
                neck = info.angle;
            worker->maximum_turn = WORK3_WALK_TURN;
            if (worker->mood == ESCAPE_MOOD)
                item->goal_anim_state = WORK3_RUN;
            else if (worker->mood == BORED_MOOD)
            {
                if (GetRandomControl() < WORK3_WAIT_CHANCE)
                {
                    item->required_anim_state = WORK3_WAIT;
                    item->goal_anim_state = WORK3_STOP;
                }
            }
            else if (info.bite && info.distance < WORK3_ATTACK0_RANGE)
                item->goal_anim_state = WORK3_STOP;
            else if (info.bite && info.distance < WORK3_ATTACK2_RANGE)
                item->goal_anim_state = WORK3_AIM2;
            else
                item->goal_anim_state = WORK3_RUN;
            break;

        case WORK3_RUN:
            if (info.ahead)
                neck = info.angle;
            worker->maximum_turn = WORK3_RUN_TURN;
            tilt = angle / 2;
            if (worker->mood == ESCAPE_MOOD)
                break;
            else if (worker->mood == BORED_MOOD)
                item->goal_anim_state = WORK3_WALK;
            else if (info.ahead && info.distance < WORK3_WALK_RANGE)
                item->goal_anim_state = WORK3_WALK;
            break;

        case WORK3_AIM0:
            if (info.ahead)
                head = info.angle;
            worker->flags = 0;
            if (info.bite && info.distance < WORK3_ATTACK0_RANGE)
                item->goal_anim_state = WORK3_PUNCH0;
            else
                item->goal_anim_state = WORK3_STOP;
            break;

        case WORK3_AIM1:
            if (info.ahead)
                head = info.angle;
            worker->flags = 0;
            if (info.ahead && info.distance < WORK3_ATTACK1_RANGE)
                item->goal_anim_state = WORK3_PUNCH1;
            else
                item->goal_anim_state = WORK3_STOP;
            break;

        case WORK3_AIM2:
            if (info.ahead)
                head = info.angle;
            worker->flags = 0;
            if (info.bite && info.distance < WORK3_ATTACK2_RANGE)
                item->goal_anim_state = WORK3_PUNCH2;
            else
                item->goal_anim_state = WORK3_WALK;
            break;

        case WORK3_PUNCH0:
            if (info.ahead)
                head = info.angle;
            if (!worker->flags && (item->touch_bits & WORK3_TOUCH))
            {
                lara_item->hit_points -= WORK3_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &work3_hit, DoBloodSplat);
                SoundEffect(72, &item->pos, 0);
                worker->flags = 1;
            }
            break;

        case WORK3_PUNCH1:
            if (info.ahead)
                head = info.angle;
            if (!worker->flags && (item->touch_bits & WORK3_TOUCH))
            {
                lara_item->hit_points -= WORK3_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &work3_hit, DoBloodSplat);
                SoundEffect(71, &item->pos, 0);
                worker->flags = 1;
            }
            if (info.ahead && info.distance > WORK3_ATTACK1_RANGE && info.distance < WORK3_ATTACK2_RANGE)
                item->goal_anim_state = WORK3_PUNCH2;
            break;

        case WORK3_PUNCH2:
            if (info.ahead)
                head = info.angle;
            if (worker->flags != 2 && (item->touch_bits & WORK3_TOUCH))
            {
                lara_item->hit_points -= WORK3_SWIPE_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &work3_hit, DoBloodSplat);
                SoundEffect(71, &item->pos, 0);
                worker->flags = 2;
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureHead(item, head);
    CreatureNeck(item, neck);

    if (item->current_anim_state < WORK3_CLIMB3)
    {
        switch (CreatureVault(item_number, angle, 2, WORK3_VAULT_SHIFT))
        {
        case 2:
            item->anim_number = objects[WORKER3].anim_index + WORK3_CLIMB1_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK3_CLIMB1;
            break;

        case 3:
            item->anim_number = objects[WORKER3].anim_index + WORK3_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK3_CLIMB2;
            break;

        case 4:
            item->anim_number = objects[WORKER3].anim_index + WORK3_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK3_CLIMB3;
            break;

        case -4:
            item->anim_number = objects[WORKER3].anim_index + WORK3_FALL3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = WORK3_FALL3;
            break;
        }
    }
    else
    {
        CreatureAnimation(item_number, angle, 0);
    }
}

#define XIAN_LORD 41
#define XIAN_LORD_STATUE 42
#define WARRIOR 43
#define WARRIOR_STATUE 44
#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotZ(PHD_ANGLE rz);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void S_PrintShadow(sint16 size, sint16 *bptr, ITEM_INFO *iptr, int unknown);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void InitInterpolate(int frac, int rate);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_RotY_I(sint16 ang);
void phd_RotX_I(sint16 ang);
void phd_RotZ_I(sint16 ang);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void phd_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);

void DrawXianLord(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 **meshpp, **jadepp;
    uint32 bit;
    sint16 *frmptr[2];
    int rate, frac;

    frac = GetFrames(item, frmptr, &rate);
    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0])))
    {
        CalculateObjectLighting(item, frmptr[0]);
        extra_rotation = (sint16 *)item->data;

        if (item->object_number == XIAN_LORD)
        {
            meshpp = &meshes[objects[XIAN_LORD].mesh_index];
            jadepp = &meshes[objects[XIAN_LORD_STATUE].mesh_index];
        }
        else
        {
            meshpp = &meshes[objects[WARRIOR].mesh_index];
            jadepp = &meshes[objects[WARRIOR_STATUE].mesh_index];
        }

        bone = bones + object->bone_index;
        bit = 1;

        if (!frac)
        {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation1 = frmptr[0] + 9;
            phd_RotYXZsuperpack(&rotation1, 0);
            phd_PutPolygons(*((bit & item->mesh_bits) ? meshpp : jadepp), clip);
            meshpp++;
            jadepp++;

            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, jadepp++)
            {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack(&rotation1, 0);
                if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
                {
                    if (poppush & ROT_Y)
                        phd_RotY(*(extra_rotation++));
                    if (poppush & ROT_X)
                        phd_RotX(*(extra_rotation++));
                    if (poppush & ROT_Z)
                        phd_RotZ(*(extra_rotation++));
                }
                bit <<= 1;
                phd_PutPolygons(*((bit & item->mesh_bits) ? meshpp : jadepp), clip);
            }
        }
        else
        {
            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8), (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            phd_PutPolygons_I(*((bit & item->mesh_bits) ? meshpp : jadepp), clip);
            meshpp++;
            jadepp++;

            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, jadepp++)
            {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();
                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
                if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
                {
                    if (poppush & ROT_Y)
                        phd_RotY_I(*(extra_rotation++));
                    if (poppush & ROT_X)
                        phd_RotX_I(*(extra_rotation++));
                    if (poppush & ROT_Z)
                        phd_RotZ_I(*(extra_rotation++));
                }
                bit <<= 1;
                phd_PutPolygons_I(*((bit & item->mesh_bits) ? meshpp : jadepp), clip);
            }
        }
    }
    phd_PopMatrix();
}

#define XIAN_TOUCH_RIGHT 0x40000
#define XIAN_TOUCH_LEFT 0x800

extern BITE_INFO xian_left_spear, xian_right_spear;

void XianDamage(ITEM_INFO *item, CREATURE_INFO *xian, sint32 damage)
{
    if (!(xian->flags & 1) && (item->touch_bits & XIAN_TOUCH_RIGHT))
    {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &xian_right_spear, DoBloodSplat);
        xian->flags |= 1;
        SoundEffect(318, &item->pos, 0);
    }
    if (!(xian->flags & 2) && (item->touch_bits & XIAN_TOUCH_LEFT))
    {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &xian_left_spear, DoBloodSplat);
        xian->flags |= 2;
        SoundEffect(318, &item->pos, 0);
    }
}

#define XIAN_START_ANIM 48
#define XIAN_START 18

void InitialiseXianLord(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == XIAN_LORD)
    {
        item->anim_number = objects[XIAN_LORD].anim_index + XIAN_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = XIAN_START;
    }
    item->status = NOT_ACTIVE;
    item->mesh_bits = 0;
}

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
void DisableBaddieAI(sint16 item_number);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

#define XIAN_STOP 1
#define XIAN_STOP2 2
#define XIAN_WALK 3
#define XIAN_RUN 4
#define XIAN_AIM1 5
#define XIAN_ATTACK1 6
#define XIAN_AIM2 7
#define XIAN_ATTACK2 8
#define XIAN_AIM3 9
#define XIAN_ATTACK3 10
#define XIAN_AIM4 11
#define XIAN_ATTACK4 12
#define XIAN_AIM5 13
#define XIAN_ATTACK5 14
#define XIAN_AIM6 15
#define XIAN_ATTACK6 16
#define XIAN_DEATH 17
#define XIAN_KILL_ANIM 49
#define XIAN_KILL 19
#define XIAN_WALK_TURN (ONE_DEGREE * 3)
#define XIAN_RUN_TURN (ONE_DEGREE * 5)
#define XIAN_CLOSE_RANGE SQUARE(WALL_L)
#define XIAN_AIM2_RANGE SQUARE(WALL_L * 3 / 2)
#define XIAN_WALK_RANGE SQUARE(WALL_L * 2)
#define XIAN_RUN_RANGE SQUARE(WALL_L * 3)
#define XIAN_DAMAGE 75
#define XIAN_BIG_DAMAGE 120
#define XIAN_BORED_CHANCE 0x200
#define XIAN_WAIT_CHANCE 0x400

void XianLordControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *xian;
    AI_INFO info;
    sint16 angle, head, neck;
    int lara_alive, random;

    if (!CreatureActive(item_number))
        return;

    head = neck = 0;
    item = &items[item_number];
    xian = (CREATURE_INFO *)item->data;
    lara_alive = (lara_item->hit_points > 0);

    if (item->hit_points <= 0)
    {
        item->current_anim_state = XIAN_DEATH;
        item->mesh_bits >>= 1;
        if (!item->mesh_bits)
        {
            SoundEffect(105, NULL, 0);
            item->mesh_bits = -1;
            item->object_number = XIAN_LORD_STATUE;
            ExplodingDeath(item_number, -1, 0);
            item->object_number = XIAN_LORD;
            DisableBaddieAI(item_number);
            KillItem(item_number);
            item->status = DEACTIVATED;
            item->flags |= ONESHOT;
        }
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 1);

        angle = CreatureTurn(item, xian->maximum_turn);

        if (item->current_anim_state != XIAN_START)
            item->mesh_bits = -1;

        switch (item->current_anim_state)
        {
        case XIAN_START:
            if (!xian->flags)
            {
                item->mesh_bits = (item->mesh_bits << 1) + 1;
                xian->flags = 3;
            }
            else
                xian->flags--;
            break;

        case XIAN_STOP:
            if (info.ahead)
                neck = info.angle;
            xian->maximum_turn = 0;

            if (xian->mood == BORED_MOOD)
            {
                random = GetRandomControl();
                if (random < XIAN_BORED_CHANCE)
                    item->goal_anim_state = XIAN_STOP2;
                else if (random < XIAN_WAIT_CHANCE)
                    item->goal_anim_state = XIAN_WALK;
            }
            else
            {
                if (info.ahead && info.distance < XIAN_CLOSE_RANGE)
                    item->goal_anim_state = XIAN_AIM1;
                else
                    item->goal_anim_state = XIAN_WALK;
            }
            break;

        case XIAN_STOP2:
            if (info.ahead)
                neck = info.angle;
            xian->maximum_turn = 0;

            if (xian->mood == ESCAPE_MOOD)
                item->goal_anim_state = XIAN_WALK;
            else if (xian->mood == BORED_MOOD)
            {
                random = GetRandomControl();
                if (random < XIAN_BORED_CHANCE)
                    item->goal_anim_state = XIAN_STOP;
                else if (random < XIAN_WAIT_CHANCE)
                    item->goal_anim_state = XIAN_WALK;
            }
            else
            {
                if (info.ahead && info.distance < XIAN_CLOSE_RANGE)
                    item->goal_anim_state = XIAN_AIM5;
                else
                    item->goal_anim_state = XIAN_WALK;
            }
            break;

        case XIAN_WALK:
            if (info.ahead)
                neck = info.angle;
            xian->maximum_turn = XIAN_WALK_TURN;

            if (xian->mood == ESCAPE_MOOD)
                item->goal_anim_state = XIAN_RUN;
            else if (xian->mood == BORED_MOOD)
            {
                random = GetRandomControl();
                if (random < XIAN_BORED_CHANCE)
                    item->goal_anim_state = XIAN_STOP;
                else if (random < XIAN_WAIT_CHANCE)
                    item->goal_anim_state = XIAN_STOP2;
            }
            else
            {
                if (info.ahead && info.distance < XIAN_WALK_RANGE)
                {
                    if (info.distance < XIAN_AIM2_RANGE)
                        item->goal_anim_state = XIAN_AIM2;
                    else if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = XIAN_AIM3;
                    else
                        item->goal_anim_state = XIAN_AIM4;
                }
                else if (!info.ahead || info.distance > XIAN_RUN_RANGE)
                    item->goal_anim_state = XIAN_RUN;
            }
            break;

        case XIAN_RUN:
            if (info.ahead)
                neck = info.angle;
            xian->maximum_turn = XIAN_RUN_TURN;

            if (xian->mood == ESCAPE_MOOD)
                break;
            else if (xian->mood == BORED_MOOD)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = XIAN_STOP;
                else
                    item->goal_anim_state = XIAN_STOP2;
            }
            else if (info.ahead && info.distance < XIAN_WALK_RANGE)
                item->goal_anim_state = XIAN_AIM6;
            break;

        case XIAN_AIM1:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_CLOSE_RANGE)
                item->goal_anim_state = XIAN_ATTACK1;
            else
                item->goal_anim_state = XIAN_STOP;
            break;

        case XIAN_AIM2:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_AIM2_RANGE)
                item->goal_anim_state = XIAN_ATTACK2;
            else
                item->goal_anim_state = XIAN_WALK;
            break;

        case XIAN_AIM3:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_WALK_RANGE)
                item->goal_anim_state = XIAN_ATTACK2;
            else
                item->goal_anim_state = XIAN_WALK;
            break;

        case XIAN_AIM4:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_WALK_RANGE)
                item->goal_anim_state = XIAN_ATTACK2;
            else
                item->goal_anim_state = XIAN_WALK;
            break;

        case XIAN_AIM5:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_CLOSE_RANGE)
                item->goal_anim_state = XIAN_ATTACK5;
            else
                item->goal_anim_state = XIAN_STOP2;
            break;

        case XIAN_AIM6:
            if (info.ahead)
                head = info.angle;
            xian->flags = 0;

            if (info.ahead && info.distance <= XIAN_WALK_RANGE)
                item->goal_anim_state = XIAN_ATTACK6;
            else
                item->goal_anim_state = XIAN_RUN;
            break;

        case XIAN_ATTACK1:
            XianDamage(item, xian, XIAN_DAMAGE);
            break;

        case XIAN_ATTACK2:
        case XIAN_ATTACK3:
        case XIAN_ATTACK4:
            if (info.ahead)
                head = info.angle;

            XianDamage(item, xian, XIAN_DAMAGE);

            if (info.ahead && info.distance < XIAN_CLOSE_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = XIAN_STOP;
                else
                    item->goal_anim_state = XIAN_STOP2;
            }
            else
                item->goal_anim_state = XIAN_WALK;
            break;

        case XIAN_ATTACK5:
            if (info.ahead)
                head = info.angle;

            XianDamage(item, xian, XIAN_DAMAGE);

            if (info.ahead && info.distance < XIAN_CLOSE_RANGE)
                item->goal_anim_state = XIAN_STOP;
            else
                item->goal_anim_state = XIAN_STOP2;
            break;

        case XIAN_ATTACK6:
            if (info.ahead)
                head = info.angle;

            XianDamage(item, xian, XIAN_BIG_DAMAGE);

            if (info.ahead && info.distance < XIAN_CLOSE_RANGE)
            {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = XIAN_STOP;
                else
                    item->goal_anim_state = XIAN_STOP2;
            }
            else if (info.ahead && info.distance < XIAN_WALK_RANGE)
                item->goal_anim_state = XIAN_WALK;
            else
                item->goal_anim_state = XIAN_RUN;
            break;
        }

        if (lara_alive && lara_item->hit_points <= 0)
            CreatureKill(item, XIAN_KILL_ANIM, XIAN_KILL, 2);
        else
        {
            CreatureTilt(item, 0);
            CreatureHead(item, head);
            CreatureNeck(item, neck);
            CreatureAnimation(item_number, angle, 0);
        }
    }
}

#define TWINKLE 239

void WarriorSparkleTrail(ITEM_INFO *item)
{
    sint32 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->object_number = TWINKLE;
        fx->pos.x_pos = item->pos.x_pos + (GetRandomDraw() << 8 >> 15) - 128;
        fx->pos.y_pos = item->pos.y_pos + (GetRandomDraw() << 8 >> 15) - 256;
        fx->pos.z_pos = item->pos.z_pos + (GetRandomDraw() << 8 >> 15) - 128;
        fx->room_number = item->room_number;
        fx->counter = -30;
        fx->frame_number = 0;
    }
    SoundEffect(312, &item->pos, 0);
}

#define WARRIOR_STOP 1
#define WARRIOR_WALK 2
#define WARRIOR_AIM1 3
#define WARRIOR_ATTACK1 4
#define WARRIOR_AIM2 5
#define WARRIOR_ATTACK2 6
#define WARRIOR_WAIT 7
#define WARRIOR_FLY 8
#define WARRIOR_STATUE_STATE 9
#define WARRIOR_AIM3 10
#define WARRIOR_ATTACK3 11
#define WARRIOR_DEATH 12
#define WARRIOR_WALK_TURN (ONE_DEGREE * 5)
#define WARRIOR_FLY_TURN (ONE_DEGREE * 4)
#define WARRIOR_CLOSE_RANGE SQUARE(WALL_L)
#define WARRIOR_WALK_RANGE SQUARE(WALL_L * 2)
#define WARRIOR_TOUCH 0xc000
#define WARRIOR_DAMAGE 300

extern BITE_INFO warrior_hit;

void WarriorControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *warrior;
    AI_INFO info;
    sint16 angle, head, neck;

    if (!CreatureActive(item_number))
        return;

    head = neck = 0;
    item = &items[item_number];
    warrior = (CREATURE_INFO *)item->data;

    if (item->hit_points <= 0)
    {
        item->current_anim_state = WARRIOR_DEATH;
        item->mesh_bits >>= 1;
        if (!item->mesh_bits)
        {
            SoundEffect(105, NULL, 0);
            item->mesh_bits = -1;
            item->object_number = WARRIOR_STATUE;
            ExplodingDeath(item_number, -1, 0);
            item->object_number = WARRIOR;
            DisableBaddieAI(item_number);
            KillItem(item_number);
            item->status = DEACTIVATED;
            item->flags |= ONESHOT;
        }
        return;
    }

    warrior->LOT.step = STEP_L;
    warrior->LOT.drop = -STEP_L;
    warrior->LOT.fly = 0;
    CreatureAIInfo(item, &info);

    if (item->current_anim_state == WARRIOR_FLY && info.zone_number != info.enemy_zone)
    {
        warrior->LOT.step = WALL_L * 20;
        warrior->LOT.drop = -WALL_L * 20;
        warrior->LOT.fly = 64;
        CreatureAIInfo(item, &info);
    }

    CreatureMood(item, &info, 1);

    angle = CreatureTurn(item, warrior->maximum_turn);

    if (item->current_anim_state != WARRIOR_STATUE_STATE)
        item->mesh_bits = -1;

    switch (item->current_anim_state)
    {
    case WARRIOR_STATUE_STATE:
        if (!warrior->flags)
        {
            item->mesh_bits = (item->mesh_bits << 1) + 1;
            warrior->flags = 3;
        }
        else
            warrior->flags--;
        break;

    case WARRIOR_STOP:
        warrior->maximum_turn = 0;
        if (info.ahead)
            neck = info.angle;

        if (lara_item->hit_points <= 0)
            item->goal_anim_state = WARRIOR_WAIT;
        else if (info.bite && info.distance < WARRIOR_CLOSE_RANGE)
        {
            if (GetRandomControl() < 0x4000)
                item->goal_anim_state = WARRIOR_AIM1;
            else
                item->goal_anim_state = WARRIOR_AIM2;
        }
        else if (info.zone_number != info.enemy_zone)
            item->goal_anim_state = WARRIOR_FLY;
        else
            item->goal_anim_state = WARRIOR_WALK;
        break;

    case WARRIOR_WALK:
        warrior->maximum_turn = WARRIOR_WALK_TURN;
        if (info.ahead)
            neck = info.angle;

        if (lara_item->hit_points <= 0)
            item->goal_anim_state = WARRIOR_STOP;
        else if (info.bite && info.distance < WARRIOR_WALK_RANGE)
            item->goal_anim_state = WARRIOR_AIM3;
        else if (info.zone_number != info.enemy_zone)
            item->goal_anim_state = WARRIOR_STOP;
        break;

    case WARRIOR_FLY:
        warrior->maximum_turn = WARRIOR_FLY_TURN;
        if (info.ahead)
            neck = info.angle;

        WarriorSparkleTrail(item);
        if (!warrior->LOT.fly)
            item->goal_anim_state = WARRIOR_STOP;
        break;

    case WARRIOR_AIM1:
        warrior->flags = 0;
        if (info.ahead)
            head = info.angle;

        if (info.bite && info.distance < WARRIOR_CLOSE_RANGE)
            item->goal_anim_state = WARRIOR_ATTACK1;
        else
            item->goal_anim_state = WARRIOR_STOP;
        break;

    case WARRIOR_AIM2:
        warrior->flags = 0;
        if (info.ahead)
            head = info.angle;

        if (info.bite && info.distance < WARRIOR_CLOSE_RANGE)
            item->goal_anim_state = WARRIOR_ATTACK2;
        else
            item->goal_anim_state = WARRIOR_STOP;
        break;

    case WARRIOR_AIM3:
        warrior->flags = 0;
        if (info.ahead)
            head = info.angle;

        if (info.bite && info.distance < WARRIOR_WALK_RANGE)
            item->goal_anim_state = WARRIOR_ATTACK3;
        else
            item->goal_anim_state = WARRIOR_WALK;
        break;

    case WARRIOR_ATTACK1:
    case WARRIOR_ATTACK2:
    case WARRIOR_ATTACK3:
        if (info.ahead)
            head = info.angle;

        if (!warrior->flags && (item->touch_bits & WARRIOR_TOUCH))
        {
            lara_item->hit_status = 1;
            lara_item->hit_points -= WARRIOR_DAMAGE;
            CreatureEffect(item, &warrior_hit, DoBloodSplat);
            warrior->flags = 1;
        }
        break;
    }

    CreatureTilt(item, 0);
    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);
}
