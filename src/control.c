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

#define DATA_TYPE 0x00ff
#define DOOR_TYPE 1
#define TILT_TYPE 2
#define ROOF_TYPE 3

extern sint16 *floor_data;

#define TRIGGER_TYPE 4
#define LAVA_TYPE 5
#define CLIMB_TYPE 6
#define END_BIT 0x8000
#define VALUE_BITS 0x03ff
#define TRIG_BITS(T) ((T & 0x3fff) >> 10)
#define TO_OBJECT 0
#define TO_CAMERA 1
#define WALL 0
#define SMALL_SLOPE 1
#define BIG_SLOPE 2

extern int height_type;
extern int IsChunkyCamera;
extern sint16 GF_NoFloor;
void S_ExitSystem(char *message);

sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    sint32 height, xoff, yoff;
    ROOM_INFO *r;
    ITEM_INFO *item;
    sint16 *data, type, trigger;

    height_type = WALL;
    while (floor->pit_room != NO_ROOM)
    {
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }

    height = floor->floor << 8;
    if (GF_NoFloor && GF_NoFloor == height)
        height = 0x4000;

    trigger_index = NULL;
    if (!floor->index)
        return height;

    data = &floor_data[floor->index];
    do
    {
        type = *(data++);
        switch (type & DATA_TYPE)
        {
        case TILT_TYPE:
            xoff = *data >> 8;
            yoff = (sint8)*data;
            if (!IsChunkyCamera || (ABS(xoff) <= 2 && ABS(yoff) <= 2))
            {
                if (ABS(xoff) > 2 || ABS(yoff) > 2)
                    height_type = BIG_SLOPE;
                else
                    height_type = SMALL_SLOPE;

                if (xoff < 0)
                    height -= (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

                if (yoff < 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
            data++;
            break;

        case DOOR_TYPE:
        case ROOF_TYPE:
            data++;
            break;

        case LAVA_TYPE:
            trigger_index = data - 1;
            break;

        case CLIMB_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            break;

        case TRIGGER_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            data++;
            do
            {
                trigger = *(data++);
                if (TRIG_BITS(trigger) != TO_OBJECT)
                {
                    if (TRIG_BITS(trigger) == TO_CAMERA)
                        trigger = *(data++);
                    continue;
                }
                item = &items[trigger & VALUE_BITS];
                if (objects[item->object_number].floor)
                    (*objects[item->object_number].floor)(item, x, y, z, &height);
            } while (!(trigger & END_BIT));
            break;

        default:
            S_ExitSystem("GetHeight(): Unknown type");
            break;
        }
    } while (!(type & END_BIT));

    return height;
}

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

#define RETURN_TARGET { target->x = x; target->y = y; target->z = z; target->room_number = last_room; return 0; }
#define RETURN_TARGET2 { target->x = x; target->y = y; target->z = z; target->room_number = room_number; return -1; }

extern int los_rooms[];
extern int number_los_rooms;

int zLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dz = target->z - start->z;
    if (dz == 0)
        return 1;

    dx = ((target->x - start->x) << WALL_SHIFT) / dz;
    dy = ((target->y - start->y) << WALL_SHIFT) / dz;

    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;

    if (dz < 0)
    {
        z = start->z & ~(WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z > target->z)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x, y, z - 1, &room_number);
            if (y > GetHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
                RETURN_TARGET;

            z -= WALL_L;
            x -= dx;
            y -= dy;
        }
    }
    else
    {
        z = start->z | (WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z < target->z)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x, y, z + 1, &room_number);
            if (y > GetHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
                RETURN_TARGET;

            z += WALL_L;
            x += dx;
            y += dy;
        }
    }

    target->room_number = room_number;
    return 1;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dx = target->x - start->x;
    if (dx == 0)
        return 1;

    dy = ((target->y - start->y) << WALL_SHIFT) / dx;
    dz = ((target->z - start->z) << WALL_SHIFT) / dx;

    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;

    if (dx < 0)
    {
        x = start->x & ~(WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x > target->x)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x - 1, y, z, &room_number);
            if (y > GetHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
                RETURN_TARGET;

            x -= WALL_L;
            y -= dy;
            z -= dz;
        }
    }
    else
    {
        x = start->x | (WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x < target->x)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x + 1, y, z, &room_number);
            if (y > GetHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
                RETURN_TARGET;

            x += WALL_L;
            y += dy;
            z += dz;
        }
    }

    target->room_number = room_number;
    return 1;
}

int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target, FLOOR_INFO *floor)
{
    sint32 dx, dy, dz, height;

    dx = target->x - start->x;
    dy = target->y - start->y;
    dz = target->z - start->z;

    height = GetHeight(floor, target->x, target->y, target->z);
    if (target->y > height && start->y < height)
    {
        target->y = height;
        target->x = start->x + dx * (height - start->y) / dy;
        target->z = start->z + dz * (height - start->y) / dy;
        return 0;
    }

    height = GetCeiling(floor, target->x, target->y, target->z);
    if (target->y < height && start->y > height)
    {
        target->y = height;
        target->x = start->x + dx * (height - start->y) / dy;
        target->z = start->z + dz * (height - start->y) / dy;
        return 0;
    }

    return 1;
}
