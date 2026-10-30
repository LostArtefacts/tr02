#include "game.h"

typedef struct {
    sint16 goal_anim_state;
    sint16 number_ranges;
    sint16 range_index;
} CHANGE_STRUCT;

typedef struct {
    sint16 start_frame;
    sint16 end_frame;
    sint16 link_anim_num;
    sint16 link_frame_num;
} RANGE_STRUCT;

extern CHANGE_STRUCT *changes;
extern RANGE_STRUCT *ranges;

int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim)
{
    int i, j;
    CHANGE_STRUCT *change;
    RANGE_STRUCT *range;

    if (item->current_anim_state == item->goal_anim_state)
        return 0;

    change = &changes[anim->change_index];
    for (i = 0; i < anim->number_changes; i++, change++)
    {
        if (change->goal_anim_state == item->goal_anim_state)
        {
            range = &ranges[change->range_index];
            for (j = 0; j < change->number_ranges; j++, range++)
            {
                if (item->frame_number >= range->start_frame && item->frame_number <= range->end_frame)
                {
                    item->anim_number = range->link_anim_num;
                    item->frame_number = range->link_frame_num;
                    return 1;
                }
            }
        }
    }
    return 0;
}

#define NO_ROOM 0xff

sint16 GetDoor(FLOOR_INFO *floor);

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number)
{
    sint32 x_floor, y_floor;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;

    r = &room[*room_number];
    do
    {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;

        if (x_floor <= 0)
        {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (x_floor >= r->x_size - 1)
        {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;

        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM)
        {
            *room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);

    if (y >= ((sint32)floor->floor << 8))
    {
        do
        {
            if (floor->pit_room == NO_ROOM)
                return floor;
            *room_number = floor->pit_room;
            r = &room[floor->pit_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y >= ((sint32)floor->floor << 8));
    }
    else if (y < ((sint32)floor->ceiling << 8))
    {
        do
        {
            if (floor->sky_room == NO_ROOM)
                return floor;
            *room_number = floor->sky_room;
            r = &room[floor->sky_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y < ((sint32)floor->ceiling << 8));
    }
    return floor;
}

#define REVERSE 0x4000

int TriggerActive(ITEM_INFO *item)
{
    int ok;

    ok = (item->flags & REVERSE) ? 0 : 1;

    if ((item->flags & CODE_BITS) != CODE_BITS)
        return !ok;

    if (!item->timer)
        return ok;

    if (item->timer == -1)
        return !ok;

    item->timer--;
    if (!item->timer)
        item->timer = -1;

    return ok;
}

#define DATA_TYPE 0x00ff
#define DOOR_TYPE 1
#define TILT_TYPE 2
#define ROOF_TYPE 3

extern sint16 *floor_data;

sint16 GetDoor(FLOOR_INFO *floor)
{
    sint16 *data, type;

    if (!floor->index)
        return NO_ROOM;

    data = &floor_data[floor->index];
    type = *(data++);

    if (type == TILT_TYPE)
    {
        data++;
        type = *(data++);
    }

    if (type == ROOF_TYPE)
    {
        data++;
        type = *(data++);
    }

    if ((type & DATA_TYPE) == DOOR_TYPE)
        return *data;
    return NO_ROOM;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int zLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target, FLOOR_INFO *floor);

int LOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int los1, los2;
    FLOOR_INFO *floor;

    if ((ABS(target->z - start->z)) > (ABS(target->x - start->x)))
    {
        los1 = xLOS(start, target);
        los2 = zLOS(start, target);
    }
    else
    {
        los1 = zLOS(start, target);
        los2 = xLOS(start, target);
    }

    if (!los2)
        return 0;

    floor = GetFloor(target->x, target->y, target->z, &target->room_number);

    if (!ClipTarget(start, target, floor) || los1 != 1 || los2 != 1)
        return 0;
    else
        return 1;
}
