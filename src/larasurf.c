#include "game.h"

void LaraSurfaceCollision(ITEM_INFO *item, COLL_INFO *coll);

void lara_col_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    LaraSurfaceCollision(item, coll);
}
