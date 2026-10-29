#ifndef LARAFUNC_H
#define LARAFUNC_H

#include "items.h"
#include "collide.h"
void lara_as_walk(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_null(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_kick(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_death(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_turn_r(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_turn_l(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stop(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_fastturn(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stepright(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stepleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_forwardjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_fastback(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_reach(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_back(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_fastturn(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_stepright(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_stepleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_slide(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_backjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_rightjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_leftjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_fallback(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_slideback(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_special(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_breath(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_airlock(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_gongbong(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_dinokill(ITEM_INFO *item, COLL_INFO *coll);

#endif
