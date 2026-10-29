#include "game.h"

void lara_as_death(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
}

void lara_as_null(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

void lara_col_turn_l(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_turn_r(item, coll);
}

void lara_col_kick(ITEM_INFO *item, COLL_INFO *coll)
{
}
