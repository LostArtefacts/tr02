#include "game.h"

extern sint16 body_bag;

void KillItem(sint16 item_number);

void InitialiseFXArray(void)
{
    int i;
    FX_INFO *fx;

    next_fx_active = NO_ITEM;
    next_fx_free = 0;
    fx = effects;
    for (i = 1; i < NUM_EFFECTS; i++, fx++)
        fx->next_fx = i;
    fx->next_fx = NO_ITEM;
}

void EffectNewRoom(sint16 fx_num, sint16 room_number)
{
    sint16 linknum;
    ROOM_INFO *r;
    FX_INFO *fx;

    fx = &effects[fx_num];
    r = &room[fx->room_number];

    linknum = r->fx_number;
    if (linknum == fx_num)
        r->fx_number = fx->next_fx;
    else
    {
        for (; linknum != NO_ITEM; linknum = effects[linknum].next_fx)
        {
            if (effects[linknum].next_fx == fx_num)
            {
                effects[linknum].next_fx = fx->next_fx;
                break;
            }
        }
    }
    fx->room_number = room_number;
    r = &room[room_number];
    fx->next_fx = r->fx_number;
    r->fx_number = fx_num;
}

void ClearBodyBag(void)
{
    sint16 item_number;
    ITEM_INFO *item;

    if (body_bag == NO_ITEM)
        return;

    item_number = body_bag;
    while (item_number != NO_ITEM)
    {
        item = &items[item_number];

        KillItem(item_number);
        item_number = item->next_active;
        item->next_active = NO_ITEM;
    }
    body_bag = NO_ITEM;
}
