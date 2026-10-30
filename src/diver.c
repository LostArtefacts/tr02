#include "game.h"
#include "box.h"

void ShootAtLara(FX_INFO *fx);

#define DIVER_HARPOON 245
#define DIVER_HARPOON_SPEED 150

sint16 Harpoon(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    sint16 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->room_number = room_number;
        fx->pos.x_rot = fx->pos.z_rot = 0;
        fx->pos.y_rot = yrot;
        fx->speed = DIVER_HARPOON_SPEED;
        fx->fallspeed = 0;
        fx->frame_number = 0;
        fx->object_number = DIVER_HARPOON;
        fx->shade = 14 * 256;
        ShootAtLara(fx);
    }
    return fx_number;
}

#define UNDERWATER 1
#define NO_ROOM 0xff

sint32 GetWaterSurface(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    r = &room[room_number];
    floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];

    if (r->flags & UNDERWATER)
    {
        while (floor->sky_room != NO_ROOM)
        {
            r = &room[floor->sky_room];
            if (!(r->flags & UNDERWATER))
                return floor->ceiling << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return NO_HEIGHT;
    }
    else
    {
        while (floor->pit_room != NO_ROOM)
        {
            r = &room[floor->pit_room];
            if (r->flags & UNDERWATER)
                return floor->floor << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
    }
    return NO_HEIGHT;
}
