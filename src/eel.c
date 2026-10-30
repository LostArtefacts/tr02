#include "game.h"
#include "box.h"

#define BIG_EEL 27
#define BIGEEL_DAMAGE 500
#define BIGEEL_ANGLE (ONE_DEGREE * 10)
#define BIGEEL_RANGE (WALL_L * 6)
#define BIGEEL_MOVE (WALL_L / 10)
#define BIGEEL_LENGTH (WALL_L * 5 / 2)
#define BIGEEL_SLIDE (BIGEEL_RANGE - BIGEEL_LENGTH)
#define BIGEEL_EMPTY 0
#define BIGEEL_ATTACK 1
#define BIGEEL_STOP 2
#define BIGEEL_DEATH 3
#define BIGEEL_DIE_ANIM 2
#define BIGEEL_TOUCH 0x180

extern BITE_INFO bigeel_bite;

void BigEelControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 angle;
    sint32 distance, x, z, pos;

    item = &items[item_number];
    pos = (sint32)item->data;
    item->pos.z_pos -= pos * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
    item->pos.x_pos -= pos * phd_sin(item->pos.y_rot) >> W2V_SHIFT;

    if (item->hit_points <= 0)
    {
        if (pos < BIGEEL_SLIDE)
            pos += BIGEEL_MOVE;

        if (item->current_anim_state != BIGEEL_DEATH)
        {
            item->anim_number = objects[BIG_EEL].anim_index + BIGEEL_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BIGEEL_DEATH;
        }
    }
    else
    {
        z = lara_item->pos.z_pos - item->pos.z_pos;
        x = lara_item->pos.x_pos - item->pos.x_pos;
        angle = phd_atan(z, x) - item->pos.y_rot;
        distance = phd_sqrt(z * z + x * x);

        switch (item->current_anim_state)
        {
        case BIGEEL_STOP:
            if (pos > 0)
                pos -= BIGEEL_MOVE;
            if (distance <= BIGEEL_RANGE && ABS(angle) < BIGEEL_ANGLE)
                item->goal_anim_state = BIGEEL_ATTACK;
            break;

        case BIGEEL_ATTACK:
            if (pos < (distance - BIGEEL_LENGTH))
                pos += BIGEEL_MOVE;
            if (!item->required_anim_state && (item->touch_bits & BIGEEL_TOUCH))
            {
                lara_item->hit_status = 1;
                lara_item->hit_points -= BIGEEL_DAMAGE;
                CreatureEffect(item, &bigeel_bite, DoBloodSplat);
                item->required_anim_state = BIGEEL_STOP;
            }
            break;
        }
    }

    item->pos.z_pos += pos * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
    item->pos.x_pos += pos * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
    item->data = (void *)pos;
    AnimateItem(item);
}
