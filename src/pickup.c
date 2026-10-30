#include "game.h"

enum switch_states { SS_OFF, SS_ON };

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
