#include "game.h"
#include "box.h"

void AlterFloorHeight(ITEM_INFO *item, int height);

void InitialiseMovingBlock(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (item->status != INVISIBLE)
        AlterFloorHeight(item, -1024);
}


int TestBlockMovable(ITEM_INFO *item, int blokhite)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (floor->floor == NO_HEIGHT / 256)
        return 1;
    if (floor->floor * 256 != item->pos.y_pos - blokhite)
        return 0;
    return 1;
}

void AlterFloorHeight(ITEM_INFO *item, int height)
{
    FLOOR_INFO *floor, *ceiling;
    sint16 room_num;

    room_num = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
    ceiling = GetFloor(item->pos.x_pos, item->pos.y_pos + height - WALL_L, item->pos.z_pos, &room_num);

    if (floor->floor == NO_HEIGHT / 256)
        floor->floor = ceiling->ceiling + height / 256;
    else
    {
        floor->floor += height / 256;
        if (floor->floor == ceiling->ceiling)
            floor->floor = NO_HEIGHT / 256;
    }

    if (boxes[floor->box].overlap_index & BLOCKABLE)
    {
        if (height < 0)
            boxes[floor->box].overlap_index |= BLOCKED;
        else
            boxes[floor->box].overlap_index &= (~BLOCKED);
    }
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
