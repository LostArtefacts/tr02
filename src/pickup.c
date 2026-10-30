#include "game.h"

enum switch_states { SS_OFF, SS_ON };

int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);
extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

#define SMALL_SWITCH 93
#define PUSH_SWITCH 103
#define AIRLOCK_SWITCH 92
#define LARA_EXTRA 12
#define EXTRA_BREATH 0
#define EXTRA_AIRLOCK 4

extern sint16 Switch1Bounds[];
extern PHD_VECTOR SmallSwitchPosition;
extern PHD_VECTOR PushSwitchPosition;
extern PHD_VECTOR AirlockPosition;

void SwitchCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
        return;

    if (laraitem->current_anim_state == AS_STOP)
    {
        if (!TestLaraPosition(Switch1Bounds, item, laraitem))
            return;
        laraitem->pos.y_rot = item->pos.y_rot;

        if (item->object_number == SMALL_SWITCH)
            AlignLaraPosition(&SmallSwitchPosition, item, laraitem);
        else if (item->object_number == PUSH_SWITCH)
            AlignLaraPosition(&PushSwitchPosition, item, laraitem);
        else if (item->object_number == AIRLOCK_SWITCH)
        {
            if (item->current_anim_state == SS_ON)
                return;
            AlignLaraPosition(&AirlockPosition, item, laraitem);
        }

        if (item->current_anim_state == SS_ON)
        {
            if (item->object_number == SMALL_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 195;
            else if (item->object_number == PUSH_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 197;
            else
                laraitem->anim_number = objects[LARA].anim_index + 63;
            laraitem->current_anim_state = AS_SWITCHON;
            item->goal_anim_state = SS_OFF;
        }
        else
        {
            laraitem->current_anim_state = AS_SWITCHOFF;
            if (item->object_number == SMALL_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 196;
            else if (item->object_number == PUSH_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 197;
            else if (item->object_number == AIRLOCK_SWITCH)
            {
                laraitem->anim_number = objects[LARA_EXTRA].anim_index;
                laraitem->frame_number = anims[laraitem->anim_number].frame_base;
                laraitem->current_anim_state = EXTRA_BREATH;
                laraitem->goal_anim_state = EXTRA_AIRLOCK;
                AnimateItem(laraitem);
                lara.extra_anim = 1;
            }
            else
                laraitem->anim_number = objects[LARA].anim_index + 64;
            item->goal_anim_state = SS_ON;
        }

        if (!lara.extra_anim)
        {
            laraitem->frame_number = anims[laraitem->anim_number].frame_base;
            laraitem->goal_anim_state = AS_STOP;
        }
        lara.gun_status = LG_HANDSBUSY;

        item->status = ACTIVE;
        AddActiveItem(item_num);
        AnimateItem(item);
    }
}

void SwitchCollision2(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.water_status != LARA_UNDERWATER || lara.gun_status != LG_ARMLESS)
        return;

    if (laraitem->current_anim_state == AS_TREAD)
    {
        if (!TestLaraPosition(Switch2Bounds, item, laraitem))
            return;

        if (item->current_anim_state == SS_ON || item->current_anim_state == SS_OFF)
        {
            if (MoveLaraPosition(&Switch2Position, item, laraitem))
            {
                laraitem->fallspeed = 0;
                laraitem->goal_anim_state = AS_SWITCHON;
                do
                {
                    AnimateLara(laraitem);
                } while (laraitem->current_anim_state != AS_SWITCHON);
                laraitem->goal_anim_state = AS_TREAD;
                lara.gun_status = LG_HANDSBUSY;
                item->status = ACTIVE;
                if (item->current_anim_state == SS_ON)
                    item->goal_anim_state = SS_OFF;
                else
                    item->goal_anim_state = SS_ON;
                AddActiveItem(item_num);
                AnimateItem(item);
            }
        }
    }
}

void SwitchControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->flags |= CODE_BITS;

    if (!TriggerActive(item))
    {
        item->goal_anim_state = SS_ON;
        item->timer = 0;
    }

    AnimateItem(item);
}

int SwitchTrigger(sint16 item_num, sint16 timer)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->object_number == AIRLOCK_SWITCH)
    {
        if (item->status == DEACTIVATED)
        {
            RemoveActiveItem(item_num);
            item->status = NOT_ACTIVE;
            return 0;
        }
        else if ((item->flags & ONESHOT) || item->current_anim_state == SS_OFF)
            return 0;
        item->flags |= ONESHOT;
        return 1;
    }

    if (item->status == DEACTIVATED)
    {
        if (item->current_anim_state == SS_OFF && timer > 0)
        {
            item->timer = timer;
            if (timer != 1)
                item->timer *= 30;
            item->status = ACTIVE;
        }
        else
        {
            RemoveActiveItem(item_num);
            item->status = NOT_ACTIVE;
        }
        return 1;
    }
    return 0;
}

int KeyTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (item->status == ACTIVE && lara.gun_status != LG_HANDSBUSY)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

int PickupTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == INVISIBLE)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

void RemoveDrawnItem(sint16 item_num);

void SecretControl(sint16 item_number)
{
    items[item_number].status = INVISIBLE;
    RemoveDrawnItem(item_number);
}
