#include "game.h"

enum switch_states { SS_OFF, SS_ON };

int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);
extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

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

#define AIRLOCK_SWITCH 92

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
