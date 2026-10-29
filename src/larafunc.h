#ifndef LARAFUNC_H
#define LARAFUNC_H

#include "items.h"
#include "collide.h"
void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_null(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_kick(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_death(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll);

#endif
