#include "game.h"

sint32 FindGridShift(sint32 src, sint32 dst)
{
    sint32 srcw, dstw;

    srcw = src >> WALL_SHIFT;
    dstw = dst >> WALL_SHIFT;
    if (srcw == dstw)
        return 0;
    src &= WALL_L - 1;
    if (dstw > srcw)
        return WALL_L - (src - 1);
    else
        return -(src + 1);
}

void GetNearByRooms(sint32 x, sint32 y, sint32 z, sint32 r, sint32 h, sint16 room_number)
{
    draw_rooms[0] = room_number;
    number_draw_rooms = 1;
    GetNewRoom(x + r, y, z + r, room_number);
    GetNewRoom(x - r, y, z + r, room_number);
    GetNewRoom(x + r, y, z - r, room_number);
    GetNewRoom(x - r, y, z - r, room_number);
    GetNewRoom(x + r, y - h, z + r, room_number);
    GetNewRoom(x - r, y - h, z + r, room_number);
    GetNewRoom(x + r, y - h, z - r, room_number);
    GetNewRoom(x - r, y - h, z - r, room_number);
}

void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    int i;

    GetFloor(x, y, z, &room_number);
    for (i = 0; i < number_draw_rooms; i++)
    {
        if (draw_rooms[i] == room_number)
            break;
    }
    if (i == number_draw_rooms)
        draw_rooms[number_draw_rooms++] = room_number;
}

void ShiftItem(ITEM_INFO *item, COLL_INFO *coll)
{
    item->pos.x_pos += coll->shift.x;
    item->pos.y_pos += coll->shift.y;
    item->pos.z_pos += coll->shift.z;
    coll->shift.x = coll->shift.y = coll->shift.z = 0;
}

void UpdateLaraRoom(ITEM_INFO *item, int height)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    sint32 x, y, z;

    x = item->pos.x_pos;
    y = item->pos.y_pos + height;
    z = item->pos.z_pos;
    room_number = item->room_number;
    floor = GetFloor(x, y, z, &room_number);
    item->floor = GetHeight(floor, x, y, z);
    if (item->room_number != room_number)
        ItemNewRoom(lara.item_number, room_number);
}

#define NO_ROOM 0xff
#define DATA_TYPE 0x00ff
#define TILT_TYPE 2

extern sint16 *floor_data;

sint16 GetTiltType(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    ROOM_INFO *r;
    sint16 *data;

    while (floor->pit_room != NO_ROOM)
    {
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }

    if (y + 512 < ((sint32)floor->floor << 8))
        return 0;

    if (floor->index)
    {
        data = &floor_data[floor->index];
        if ((*data & DATA_TYPE) == TILT_TYPE)
            return *(data + 1);
    }

    return 0;
}

void EffectSpaz(ITEM_INFO *laraitem, COLL_INFO *coll)
{
    sint32 x, z;
    PHD_ANGLE hitang;

    x = lara.spaz_effect->pos.x_pos - laraitem->pos.x_pos;
    z = lara.spaz_effect->pos.z_pos - laraitem->pos.z_pos;
    hitang = laraitem->pos.y_rot - (32768 + phd_atan(z, x));
    lara.hit_direction = (uint16)(hitang + 0x2000) / 0x4000;
    if (!lara.hit_frame)
        SoundEffect(31, &laraitem->pos, 0);

    lara.hit_frame++;
    if (lara.hit_frame > 34)
        lara.hit_frame = 34;

    lara.spaz_effect_count--;
}

void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;

    if (coll->enable_baddie_push && lara.water_status != LARA_UNDERWATER && !lara.water_status != LARA_SURFACE)
        ItemPushLara(item, laraitem, coll, coll->enable_spaz, 0);
}

void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;
    if (coll->enable_baddie_push)
        ItemPushLara(item, laraitem, coll, 0, 1);
}

void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;

    if (coll->enable_baddie_push)
    {
        if (item->current_anim_state != item->goal_anim_state)
            ItemPushLara(item, laraitem, coll, coll->enable_spaz, 1);
        else
            ItemPushLara(item, laraitem, coll, 0, 1);
    }
}

void TrapCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == ACTIVE)
    {
        if (!TestBoundsCollide(item, laraitem, coll->radius))
            return;
        TestCollision(item, laraitem);
    }
    else if (item->status != INVISIBLE)
        ObjectCollision(item_num, laraitem, coll);
}

int Move3DPosTo3DPos(PHD_3DPOS *srcpos, PHD_3DPOS *destpos, int velocity, PHD_ANGLE angadd)
{
    int dist, x, y, z;
    PHD_ANGLE angdif;

    x = destpos->x_pos - srcpos->x_pos;
    y = destpos->y_pos - srcpos->y_pos;
    z = destpos->z_pos - srcpos->z_pos;
    dist = phd_sqrt(x * x + y * y + z * z);
    if (velocity >= dist)
    {
        srcpos->x_pos = destpos->x_pos;
        srcpos->y_pos = destpos->y_pos;
        srcpos->z_pos = destpos->z_pos;
    }
    else
    {
        srcpos->x_pos += (x * velocity) / dist;
        srcpos->y_pos += (y * velocity) / dist;
        srcpos->z_pos += (z * velocity) / dist;
    }
    angdif = destpos->x_rot - srcpos->x_rot;
    if (angdif > angadd)
        srcpos->x_rot += angadd;
    else if (angdif < -angadd)
        srcpos->x_rot -= angadd;
    else
        srcpos->x_rot = destpos->x_rot;
    angdif = destpos->y_rot - srcpos->y_rot;
    if (angdif > angadd)
        srcpos->y_rot += angadd;
    else if (angdif < -angadd)
        srcpos->y_rot -= angadd;
    else
        srcpos->y_rot = destpos->y_rot;
    angdif = destpos->z_rot - srcpos->z_rot;
    if (angdif > angadd)
        srcpos->z_rot += angadd;
    else if (angdif < -angadd)
        srcpos->z_rot -= angadd;
    else
        srcpos->z_rot = destpos->z_rot;
    if (srcpos->x_pos == destpos->x_pos && srcpos->y_pos == destpos->y_pos && srcpos->z_pos == destpos->z_pos &&
        srcpos->x_rot == destpos->x_rot && srcpos->y_rot == destpos->y_rot && srcpos->z_rot == destpos->z_rot)
        return 1;
    else
        return 0;
}
