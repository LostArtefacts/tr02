#include "game.h"

#define FALLING_PLANK 57
#define FLAME 252

sint16 CreateEffect(sint16 room_num);
int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z);

enum trap_anims { TRAP_SET, TRAP_ACTIVATE, TRAP_WORKING, TRAP_FINISHED };
enum door_anims { DOOR_CLOSED, DOOR_OPEN };

void TrapDoorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (!OnTrapDoor(item, x, z))
        return;
    if (y > item->pos.y_pos)
    {
        if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos > *height)
            *height = item->pos.y_pos + STEP_L;
    }
}

int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z)
{
    int tx, tz;

    x >>= WALL_SHIFT;
    z >>= WALL_SHIFT;
    tx = item->pos.x_pos >> WALL_SHIFT;
    tz = item->pos.z_pos >> WALL_SHIFT;

    if (item->pos.y_rot == 0 && x == tx && (z == tz || z == tz + 1))
        return (1);
    else if (item->pos.y_rot == -0x8000 && x == tx && (z == tz || z == tz - 1))
        return (1);
    else if (item->pos.y_rot == 0x4000 && z == tz && (x == tx || x == tx + 1))
        return (1);
    else if (item->pos.y_rot == -0x4000 && z == tz && (x == tx || x == tx - 1))
        return (1);

    return (0);
}

void FallingBlockFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 origin;

    if (item->object_number == FALLING_PLANK)
        origin = WALL_L;
    else
        origin = STEP_L * 2;

    if (y <= item->pos.y_pos - origin)
        if (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE)
            *height = item->pos.y_pos - origin;
}

void FallingBlockCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 origin;

    if (item->object_number == FALLING_PLANK)
        origin = WALL_L;
    else
        origin = STEP_L * 2;

    if (y > item->pos.y_pos - origin)
        if (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE)
            *height = item->pos.y_pos + STEP_L - origin;
}

void LaraBurn(void)
{
    sint16 fx_number;
    FX_INFO *fx;

    if (lara.burn)
        return;

    fx_number = CreateEffect(lara_item->room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->frame_number = 0;
        fx->object_number = FLAME;
        fx->counter = -1;
        lara.burn = 1;
    }
}
