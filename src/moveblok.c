#include "game.h"

void AlterFloorHeight(ITEM_INFO *item, int height);

void InitialiseMovingBlock(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (item->status != INVISIBLE)
        AlterFloorHeight(item, -1024);
}

void DrawMovableBlock(ITEM_INFO *item)
{
    if (item->status != ACTIVE)
        DrawAnimatingItem(item);
    else
        DrawUnclippedItem(item);
}

void DrawUnclippedItem(ITEM_INFO *item)
{
    int left, right, top, bottom;

    left = phd_left;
    right = phd_right;
    top = phd_top;
    bottom = phd_bottom;
    phd_left = phd_top = 0;
    phd_bottom = phd_winymax;
    phd_right = phd_winxmax;
    DrawAnimatingItem(item);
    phd_left = left;
    phd_right = right;
    phd_top = top;
    phd_bottom = bottom;
}
