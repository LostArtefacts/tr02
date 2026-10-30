#include <string.h>
#include "game.h"
#include "box.h"

typedef struct {
    FLOOR_INFO *floor;
    FLOOR_INFO data;
    sint16 block;
} DOORPOS_DATA;

enum door_anims { DOOR_CLOSED, DOOR_OPEN };

typedef struct {
    sint32 start_height;
    sint32 wait_time;
} LIFT_INFO;

void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void *game_malloc(int size, int type);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
int ItemNearLara(PHD_3DPOS *pos, sint32 distance);

extern sint16 DeathSlideBounds[12];
extern PHD_VECTOR DeathSlidePosition;

#define NO_ROOM 0xff
#define FINAL_COUNTDOWN 150
#define BELL_STOP 0
#define BELL_SWING 1

extern sint16 final_boss_active;

void ControlCutShotgun(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (final_boss_active >= FINAL_COUNTDOWN)
    {
        item->status = ACTIVE;
        AnimateItem(item);
    }
    else
        item->status = INVISIBLE;
}

void DeathSlideCollision(sint16 item_number, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    if (!(input & IN_ACTION) || laraitem->gravity_status || lara.gun_status != LG_ARMLESS
        || laraitem->current_anim_state != AS_STOP)
        return;

    item = &items[item_number];
    if (item->status != NOT_ACTIVE)
        return;

    if (TestLaraPosition(DeathSlideBounds, item, laraitem))
    {
        AlignLaraPosition(&DeathSlidePosition, item, laraitem);
        lara.gun_status = LG_HANDSBUSY;

        laraitem->goal_anim_state = AS_DEATHSLIDE;
        do
            AnimateItem(laraitem);
        while (laraitem->current_anim_state != AS_NULL);

        if (!item->active)
            AddActiveItem(item_number);
        item->status = ACTIVE;
        item->flags |= ONESHOT;
    }
}

void BellControl(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;

    item = &items[item_number];

    item->goal_anim_state = BELL_SWING;

    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    TestTriggers(trigger_index, 1);

    AnimateItem(item);

    if (item->current_anim_state == BELL_STOP)
    {
        item->status = NOT_ACTIVE;
        RemoveActiveItem(item_number);
    }
}

void InitialiseWindow(sint16 item_number)
{
    ITEM_INFO *item;
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    item = &items[item_number];
    item->flags = 0;

    item->mesh_bits = 0x1;

    r = &room[item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    if (boxes[floor->box].overlap_index & BLOCKABLE)
        boxes[floor->box].overlap_index |= BLOCKED;
}

void SmashWindow(sint16 item_number)
{
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    ITEM_INFO *item;

    item = &items[item_number];

    r = &room[item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    if (boxes[floor->box].overlap_index & BLOCKABLE)
        boxes[floor->box].overlap_index &= (~BLOCKED);

    SoundEffect(58, &item->pos, 0);

    item->collidable = 0;
    item->mesh_bits = 0x0fffe;
    ExplodingDeath(item_number, 0x0fefe, 0);

    item->flags |= ONESHOT;
    if (item->status == ACTIVE)
        RemoveActiveItem(item_number);
    item->status = DEACTIVATED;
}

void WindowControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 speed;

    item = &items[item_number];
    if (item->flags & ONESHOT)
        return;

    if (lara.skidoo == NO_ITEM)
    {
        if (!item->touch_bits)
            return;
        item->touch_bits = 0;
        speed = lara_item->speed * phd_cos((sint16)(lara_item->pos.y_rot - item->pos.y_rot)) >> W2V_SHIFT;
        if (ABS(speed) < 50)
            return;
    }
    else if (!ItemNearLara(&item->pos, WALL_L / 2))
        return;

    SmashWindow(item_number);
}

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

void InitialiseLift(sint16 item_number)
{
    LIFT_INFO *lift;

    items[item_number].data = lift = (LIFT_INFO *)game_malloc(sizeof(LIFT_INFO), 0);

    lift->start_height = items[item_number].pos.y_pos;
    lift->wait_time = 0;
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

void BridgeTilt2Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 1);
    if (y <= level)
        *height = level;
}

void BridgeTilt2Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 1);
    if (y > level)
        *height = level + STEP_L;
}
