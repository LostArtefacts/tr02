#include <string.h>
#include "game.h"
#include "box.h"

typedef struct {
    FLOOR_INFO *floor;
    FLOOR_INFO data;
    sint16 block;
} DOORPOS_DATA;

enum door_anims { DOOR_CLOSED, DOOR_OPEN };

void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

#define NO_ROOM 0xff
#define NO_HEIGHT (-0x7f00)

void ShutThatDoor(DOORPOS_DATA *d)
{
    FLOOR_INFO *floor;

    if (d->floor)
    {
        floor = d->floor;
        floor->index = 0;
        floor->box = NO_BOX;
        floor->floor = floor->ceiling = NO_HEIGHT >> 8;
        floor->pit_room = floor->sky_room = NO_ROOM;

        if (d->block != NO_BOX)
            boxes[d->block].overlap_index |= BLOCKED;
    }
}

void OpenThatDoor(DOORPOS_DATA *d)
{
    if (d->floor)
    {
        memcpy(d->floor, &d->data, sizeof(FLOOR_INFO));

        if (d->block != NO_BOX)
            boxes[d->block].overlap_index &= (~BLOCKED);
    }
}

int OnDrawBridge(ITEM_INFO *item, sint32 x, sint32 y)
{
    int ix, iy;

    ix = item->pos.z_pos >> WALL_SHIFT;
    iy = item->pos.x_pos >> WALL_SHIFT;

    x >>= WALL_SHIFT;
    y >>= WALL_SHIFT;

    if (item->pos.y_rot == 0 && y == iy && (x == ix - 1 || x == ix - 2))
        return (1);
    else if (item->pos.y_rot == -0x8000 && y == iy && (x == ix + 1 || x == ix + 2))
        return (1);
    else if (item->pos.y_rot == 0x4000 && x == ix && (y == iy - 1 || y == iy - 2))
        return (1);
    else if (item->pos.y_rot == -0x4000 && x == ix && (y == iy + 1 || y == iy + 2))
        return (1);

    return (0);
}

void DrawBridgeFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (item->current_anim_state == DOOR_OPEN)
    {
        if (!OnDrawBridge(item, z, x))
            return;

        if (y <= item->pos.y_pos)
            *height = item->pos.y_pos;
    }
}

void DrawBridgeCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (item->current_anim_state == DOOR_OPEN)
    {
        if (!OnDrawBridge(item, z, x))
            return;

        if (y > item->pos.y_pos)
            *height = item->pos.y_pos + STEP_L;
    }
}

void DrawBridgeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->current_anim_state == DOOR_CLOSED)
        DoorCollision(item_num, laraitem, coll);
}

void LiftFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);

    if (floor < *height)
        *height = floor;
}

void LiftCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);
    if (ceiling > *height)
        *height = ceiling;
}

void BridgeFlatFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y <= item->pos.y_pos)
        *height = item->pos.y_pos;
}

void BridgeFlatCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y > item->pos.y_pos)
        *height = item->pos.y_pos + STEP_L;
}

sint32 GetOffset(ITEM_INFO *item, sint32 x, sint32 z)
{
    if (item->pos.y_rot == 0)
        return ((WALL_L - x) & (WALL_L - 1));
    else if (item->pos.y_rot == -0x8000)
        return (x & (WALL_L - 1));
    else if (item->pos.y_rot == 0x4000)
        return (z & (WALL_L - 1));
    return ((WALL_L - z) & (WALL_L - 1));
}

void BridgeTilt1Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 2);
    if (y <= level)
        *height = level;
}

void BridgeTilt1Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 2);
    if (y > level)
        *height = level + STEP_L;
}

void BridgeTilt2Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 1);
    if (y > level)
        *height = level + STEP_L;
}
