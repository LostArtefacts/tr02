#include "game.h"

void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
}

void lara_as_null(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
}

void lara_col_kick(ITEM_INFO *item, COLL_INFO *coll)
{
}
