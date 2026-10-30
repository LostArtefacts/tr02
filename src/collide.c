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

typedef struct {
    sint32 x, y, z;
    PHD_ANGLE y_rot;
    sint16 shade;
    sint16 shadeB;
    sint16 static_number;
} MESH_INFO;

typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

extern STATIC_INFO StaticObjects[];

enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

int CollideStaticObjects(COLL_INFO *coll, sint32 x, sint32 y, sint32 z, sint16 room_number, sint32 hite)
{
    int i, j;
    ROOM_INFO *r;
    MESH_INFO *mesh;
    STATIC_INFO *sinfo;
    sint32 xmin, xmax, ymin, ymax, zmin, zmax;
    sint32 inxmin, inxmax, inymin, inymax, inzmin, inzmax;
    PHD_VECTOR shifter;
    sint32 shl, shr;

    coll->hit_static = 0;
    inxmin = x - coll->radius;
    inxmax = x + coll->radius;
    inymin = y - hite;
    inymax = y;
    inzmin = z - coll->radius;
    inzmax = z + coll->radius;
    shifter.x = shifter.y = shifter.z = 0;
    GetNearByRooms(x, y, z, coll->radius + 50, hite + 50, room_number);
    for (i = 0; i < number_draw_rooms; i++)
    {
        r = &room[draw_rooms[i]];
        mesh = (MESH_INFO *)r->mesh;

        for (j = r->num_meshes; j > 0; j--, mesh++)
        {
            sinfo = &StaticObjects[mesh->static_number];
            if (sinfo->flags & 1)
                continue;

            ymin = mesh->y + sinfo->y_minc;
            ymax = mesh->y + sinfo->y_maxc;
            switch (mesh->y_rot)
            {
            default:
                xmin = mesh->x + sinfo->x_minc;
                xmax = mesh->x + sinfo->x_maxc;
                zmin = mesh->z + sinfo->z_minc;
                zmax = mesh->z + sinfo->z_maxc;
                break;

            case 16384:
                xmin = mesh->x + sinfo->z_minc;
                xmax = mesh->x + sinfo->z_maxc;
                zmin = mesh->z - sinfo->x_maxc;
                zmax = mesh->z - sinfo->x_minc;
                break;

            case -32768:
                xmin = mesh->x - sinfo->x_maxc;
                xmax = mesh->x - sinfo->x_minc;
                zmin = mesh->z - sinfo->z_maxc;
                zmax = mesh->z - sinfo->z_minc;
                break;

            case -16384:
                xmin = mesh->x - sinfo->z_maxc;
                xmax = mesh->x - sinfo->z_minc;
                zmin = mesh->z + sinfo->x_minc;
                zmax = mesh->z + sinfo->x_maxc;
                break;
            }

            if (inxmax <= xmin || inxmin >= xmax || inymax <= ymin || inymin >= ymax || inzmax <= zmin || inzmin >= zmax)
                continue;

            shl = inxmax - xmin;
            shr = xmax - inxmin;
            if (shl < shr)
                shifter.x = -shl;
            else
                shifter.x = shr;

            shl = inzmax - zmin;
            shr = zmax - inzmin;
            if (shl < shr)
                shifter.z = -shl;
            else
                shifter.z = shr;

            switch (coll->quadrant)
            {
            case NORTH:
                if (shifter.x > coll->radius || shifter.x < -coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = coll->old.x - x;
                    coll->coll_type = COLL_FRONT;
                }
                else if (shifter.x > 0 && shifter.x <= coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = 0;
                    coll->coll_type = COLL_LEFT;
                }
                else if (shifter.x < 0 && shifter.x >= -coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = 0;
                    coll->coll_type = COLL_RIGHT;
                }
                break;

            case EAST:
                if (shifter.z > coll->radius || shifter.z < -coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = coll->old.z - z;
                    coll->coll_type = COLL_FRONT;
                }
                else if (shifter.z > 0 && shifter.z <= coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = 0;
                    coll->coll_type = COLL_RIGHT;
                }
                else if (shifter.z < 0 && shifter.z >= -coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = 0;
                    coll->coll_type = COLL_LEFT;
                }
                break;

            case SOUTH:
                if (shifter.x > coll->radius || shifter.x < -coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = coll->old.x - x;
                    coll->coll_type = COLL_FRONT;
                }
                else if (shifter.x > 0 && shifter.x <= coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = 0;
                    coll->coll_type = COLL_RIGHT;
                }
                else if (shifter.x < 0 && shifter.x >= -coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = 0;
                    coll->coll_type = COLL_LEFT;
                }
                break;

            case WEST:
                if (shifter.z > coll->radius || shifter.z < -coll->radius)
                {
                    coll->shift.x = shifter.x;
                    coll->shift.z = coll->old.z - z;
                    coll->coll_type = COLL_FRONT;
                }
                else if (shifter.z > 0 && shifter.z <= coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = 0;
                    coll->coll_type = COLL_LEFT;
                }
                else if (shifter.z < 0 && shifter.z >= -coll->radius)
                {
                    coll->shift.z = shifter.z;
                    coll->shift.x = 0;
                    coll->coll_type = COLL_RIGHT;
                }
                break;
            }
            coll->hit_static = 1;
            return 1;
        }
    }
    return 0;
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

#define TARGET_DIST (WALL_L * 4)

extern sint16 InventoryChosen;

void EffectSpaz(ITEM_INFO *laraitem, COLL_INFO *coll);

void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll)
{
    int i, x, y, z;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    sint16 roomies[20], numroom, *door;
    sint16 item_num, nex;

    laraitem->hit_status = 0;
    lara.hit_direction = -1;
    if (laraitem->hit_points <= 0)
        return;
    numroom = 1;
    roomies[0] = laraitem->room_number;
    door = room[laraitem->room_number].door;
    if (door)
    {
        for (i = (int)*(door++); i > 0; i--)
        {
            roomies[numroom++] = *door;
            door += 16;
        }
    }

    for (i = 0; i < numroom; i++)
    {
        item_num = room[roomies[i]].item_number;
        while (item_num != NO_ITEM)
        {
            item = &items[item_num];
            nex = item->next_item;
            if (item->collidable && item->status != INVISIBLE)
            {
                object = &objects[item->object_number];
                if (object->collision)
                {
                    x = laraitem->pos.x_pos - item->pos.x_pos;
                    y = laraitem->pos.y_pos - item->pos.y_pos;
                    z = laraitem->pos.z_pos - item->pos.z_pos;
                    if (x > -TARGET_DIST && x < TARGET_DIST &&
                        z > -TARGET_DIST && z < TARGET_DIST &&
                        y > -TARGET_DIST && y < TARGET_DIST)
                        (*object->collision)(item_num, laraitem, coll);
                }
            }
            item_num = nex;
        }
    }

    if (lara.spaz_effect_count)
        EffectSpaz(laraitem, coll);
    if (lara.hit_direction == -1)
        lara.hit_frame = 0;
    InventoryChosen = -1;
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

sint16 *GetBestFrame(ITEM_INFO *item);
void GetCollisionInfo(COLL_INFO *coll, sint32 xpos, sint32 ypos, sint32 zpos, sint16 room_number, sint32 objheight);

void ItemPushLara(ITEM_INFO *item, ITEM_INFO *laraitem, COLL_INFO *coll, int spazon, int bigpush)
{
    int minx, maxx, minz, maxz;
    int x, z, rx, rz, c, s;
    PHD_ANGLE hitang;
    int l, r, t, b, ax, az;
    OBJECT_INFO *object;
    sint16 *bounds;

    x = laraitem->pos.x_pos - item->pos.x_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    rx = (c * x - s * z) >> W2V_SHIFT;
    rz = (c * z + s * x) >> W2V_SHIFT;

    object = &objects[item->object_number];
    bounds = GetBestFrame(item);
    minx = bounds[0];
    maxx = bounds[1];
    minz = bounds[4];
    maxz = bounds[5];
    if (bigpush)
    {
        minx -= coll->radius;
        maxx += coll->radius;
        minz -= coll->radius;
        maxz += coll->radius;
    }
    if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz)
    {
        l = rx - minx;
        r = maxx - rx;
        t = maxz - rz;
        b = rz - minz;

        if (l <= r && l <= t && l <= b)
            rx -= l;
        else if (r <= l && r <= t && r <= b)
            rx += r;
        else if (t <= l && t <= r && t <= b)
            rz += t;
        else
            rz -= b;
        ax = (c * rx + s * rz) >> W2V_SHIFT;
        az = (c * rz - s * rx) >> W2V_SHIFT;

        laraitem->pos.x_pos = item->pos.x_pos + ax;
        laraitem->pos.z_pos = item->pos.z_pos + az;

        rx = (bounds[0] + bounds[1]) / 2;
        rz = (bounds[4] + bounds[5]) / 2;
        x -= (c * rx + s * rz) >> W2V_SHIFT;
        z -= (c * rz - s * rx) >> W2V_SHIFT;
        if (spazon && bounds[3] - bounds[2] > STEP_L)
        {
            hitang = laraitem->pos.y_rot - (32768 + phd_atan(z, x));
            lara.hit_direction = (uint16)(hitang + 0x2000) / 0x4000;
            if (!lara.hit_frame)
                SoundEffect(31, &laraitem->pos, 0);
            lara.hit_frame++;
            if (lara.hit_frame > 34)
                lara.hit_frame = 34;
        }
        coll->bad_pos = NO_BAD_POS;
        coll->bad_neg = -384;
        coll->bad_ceiling = 0;
        hitang = coll->facing;
        coll->facing = phd_atan(laraitem->pos.z_pos - coll->old.z, laraitem->pos.x_pos - coll->old.x);
        GetCollisionInfo(coll, laraitem->pos.x_pos, laraitem->pos.y_pos, laraitem->pos.z_pos, laraitem->room_number, LARA_HITE);
        coll->facing = hitang;
        if (coll->coll_type != COLL_NONE)
        {
            laraitem->pos.x_pos = coll->old.x;
            laraitem->pos.z_pos = coll->old.z;
        }
        else
        {
            coll->old.x = laraitem->pos.x_pos;
            coll->old.y = laraitem->pos.y_pos;
            coll->old.z = laraitem->pos.z_pos;
            UpdateLaraRoom(laraitem, -10);
        }
    }
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius)
{
    OBJECT_INFO *object;
    sint32 c, s, rx, rz, x, z;
    sint16 *larabounds, *bounds;
    sint32 minx, maxx, minz, maxz;

    object = &objects[item->object_number];
    bounds = GetBestFrame(item);
    object = &objects[laraitem->object_number];
    larabounds = GetBestFrame(laraitem);

    if (item->pos.y_pos + *(bounds + 3) <= laraitem->pos.y_pos + *(larabounds + 2)
        || item->pos.y_pos + *(bounds + 2) >= laraitem->pos.y_pos + *(larabounds + 3))
        return 0;

    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    x = laraitem->pos.x_pos - item->pos.x_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    rx = (c * x - s * z) >> W2V_SHIFT;
    rz = (c * z + s * x) >> W2V_SHIFT;
    minx = *(bounds + 0) - radius;
    maxx = *(bounds + 1) + radius;
    minz = *(bounds + 4) - radius;
    maxz = *(bounds + 5) + radius;
    if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz)
        return 1;
    return 0;
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
