#include <string.h>
#include "game.h"
#include "box.h"

typedef struct {
    FLOOR_INFO *floor;
    FLOOR_INFO data;
    sint16 block;
} DOORPOS_DATA;

typedef struct {
    DOORPOS_DATA d1, d1flip, d2, d2flip;
} DOOR_DATA;

enum door_anims { DOOR_CLOSED, DOOR_OPEN };

typedef struct {
    sint32 start_height;
    sint32 wait_time;
} LIFT_INFO;

void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void *game_malloc(int size, int type);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
int ItemNearLara(PHD_3DPOS *pos, sint32 distance);
void S_CDPlay(sint16 track, int mode);
void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void AnimateLara(ITEM_INFO *item);

extern sint16 DeathSlideBounds[12];
extern PHD_VECTOR DeathSlidePosition;
extern int flipmap[];

#define NO_ROOM 0xff
#define FINAL_COUNTDOWN 150
#define BELL_STOP 0
#define BELL_SWING 1
#define BOWL_POUR 1
#define HOT_LIQUID 220
#define LIFT_WAIT_TIME 90
#define FALLING_CEILING1 79
#define FLAME_EMITTER 253
#define DEATH_SPEED 100
#define DEATH_ACC 5
#define DEATH_GRAB 1

extern sint16 final_boss_active;

void EarthQuake(sint16 item_number)
{
    sint16 earth_item;
    int want;
    ITEM_INFO *item;

    item = &items[item_number];

    if (GetRandomDraw() < 0x200)
    {
        SoundEffect(298, NULL, 0);
        camera.bounce = -200;
    }

    want = GetRandomControl();
    if (want < 0x400)
    {
        want = (want < 0x200) ? FLAME_EMITTER : FALLING_CEILING1;
        for (earth_item = room[item->room_number].item_number; earth_item != NO_ITEM; earth_item = items[earth_item].next_item)
        {
            item = &items[earth_item];
            if (item->object_number != want)
                continue;
            if (item->status == ACTIVE || item->status == DEACTIVATED)
                continue;
            AddActiveItem(earth_item);
            item->status = ACTIVE;
            item->flags = CODE_BITS;
            item->timer = 0;
            return;
        }
    }
}

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

void MiniCopterControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    PHD_3DPOS pos;

    item = &items[item_number];

    item->pos.z_pos += 100;

    pos.x_pos = lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 2);
    pos.y_pos = lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 2);
    pos.z_pos = lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 2);
    SoundEffect(297, &pos, 0);

    if (ABS(item->pos.z_pos - lara_item->pos.z_pos) > WALL_L * 30)
        KillItem(item_number);

    AnimateItem(item);

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
}

void InitialiseDyingMonk(sint16 item_number)
{
    int *who;
    ITEM_INFO *item;

    item = &items[item_number];

    item->data = who = game_malloc(sizeof(int) * 2, 0);

    item_number = room[item->room_number].item_number;
    while (item_number != NO_ITEM)
    {
        if (objects[items[item_number].object_number].intelligent)
            *(who++) = item_number;

        item_number = items[item_number].next_item;
    }
}

void DyingMonk(sint16 item_number)
{
    int *who;
    ITEM_INFO *item;

    item = &items[item_number];
    who = (int *)item->data;

    if (items[*who].hit_points <= 0 && items[*(who + 1)].hit_points <= 0)
    {
        if (ABS(lara_item->pos.x_pos - item->pos.x_pos) < WALL_L && ABS(lara_item->pos.z_pos - item->pos.z_pos) < WALL_L
            && !lara_item->gravity_status && lara_item->pos.y_pos == item->pos.y_pos)
            level_complete = 1;
    }
}

void ControlGongBonger(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    FLOOR_INFO *floor;

    item = &items[item_number];

    AnimateItem(item);

    if (item->frame_number - anims[item->anim_number].frame_base == 41)
    {
        S_CDPlay(61, 0);
        camera.bounce -= 50;
    }

    if (item->frame_number == anims[item->anim_number].frame_end)
    {
        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        TestTriggers(trigger_index, 1);

        KillItem(item_number);
    }
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

void ControlDeathSlide(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 c, s, x, y, z;
    sint16 room_number;
    int lara_on_deathslide;
    GAME_VECTOR *old;

    item = &items[item_number];
    if (item->status == ACTIVE)
    {
        if (!(item->flags & ONESHOT))
        {
            old = (GAME_VECTOR *)item->data;
            item->pos.x_pos = old->x;
            item->pos.y_pos = old->y;
            item->pos.z_pos = old->z;
            if (old->room_number != item->room_number)
                ItemNewRoom(item_number, old->room_number);
            item->status = NOT_ACTIVE;
            item->current_anim_state = item->goal_anim_state = DEATH_GRAB;
            item->anim_number = objects[item->object_number].anim_index;
            item->frame_number = anims[item->anim_number].frame_base;
            RemoveActiveItem(item_number);
            return;
        }

        if (item->current_anim_state == DEATH_GRAB)
        {
            AnimateItem(item);
            return;
        }

        AnimateItem(item);

        if (item->fallspeed < DEATH_SPEED)
            item->fallspeed += DEATH_ACC;

        c = phd_cos(item->pos.y_rot);
        s = phd_sin(item->pos.y_rot);

        item->pos.z_pos += item->fallspeed * c >> W2V_SHIFT;
        item->pos.x_pos += item->fallspeed * s >> W2V_SHIFT;
        item->pos.y_pos += item->fallspeed >> 2;
        room_number = item->room_number;
        GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        if (room_number != item->room_number)
            ItemNewRoom(item_number, room_number);

        lara_on_deathslide = (lara_item->current_anim_state == AS_DEATHSLIDE);
        if (lara_on_deathslide)
        {
            lara_item->pos.x_pos = item->pos.x_pos;
            lara_item->pos.y_pos = item->pos.y_pos;
            lara_item->pos.z_pos = item->pos.z_pos;
        }

        z = item->pos.z_pos + (WALL_L * c >> W2V_SHIFT);
        x = item->pos.x_pos + (WALL_L * s >> W2V_SHIFT);
        y = item->pos.y_pos + (STEP_L >> 2);
        floor = GetFloor(x, y, z, &room_number);
        if (GetHeight(floor, x, y, z) <= y + STEP_L || GetCeiling(floor, x, y, z) >= y - STEP_L)
        {
            if (lara_on_deathslide)
            {
                lara_item->goal_anim_state = AS_FORWARDJUMP;
                AnimateLara(lara_item);
                lara_item->gravity_status = 1;
                lara_item->speed = item->fallspeed;
                lara_item->fallspeed = item->fallspeed >> 2;
            }

            SoundEffect(281, &item->pos, 0);
            RemoveActiveItem(item_number);
            item->status = NOT_ACTIVE;
            item->flags -= ONESHOT;
        }
        else
            SoundEffect(280, &item->pos, 0);
    }
}

void BigBowlControl(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];

    if (item->current_anim_state == BOWL_POUR)
    {
        fx_number = CreateEffect(item->room_number);
        if (fx_number != NO_ITEM)
        {
            fx = &effects[fx_number];
            fx->object_number = HOT_LIQUID;
            fx->pos.x_pos = item->pos.x_pos + STEP_L * 2;
            fx->pos.z_pos = item->pos.z_pos + STEP_L * 2;
            fx->pos.y_pos = item->pos.y_pos + STEP_L * 2 + 100;
            fx->room_number = item->room_number;
            fx->frame_number = GetRandomDraw() * objects[HOT_LIQUID].nmeshes >> 15;
            fx->fallspeed = 0;
            fx->shade = 0x800;
        }

        item->timer++;
        if (item->timer == 30 * 5)
        {
            flipmap[4] = CODE_BITS | ONESHOT;
            if (!flip_status)
                FlipMap();
        }
    }

    AnimateItem(item);

    if (item->status == DEACTIVATED && item->timer >= 30 * 7)
        RemoveActiveItem(item_number);
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

void DoorControl(sint16 item_number)
{
    ITEM_INFO *item;
    DOOR_DATA *door;

    item = &items[item_number];
    door = (DOOR_DATA *)item->data;

    if (TriggerActive(item))
    {
        if (item->current_anim_state == DOOR_CLOSED)
            item->goal_anim_state = DOOR_OPEN;
        else
        {
            OpenThatDoor(&door->d1);
            OpenThatDoor(&door->d2);
            OpenThatDoor(&door->d1flip);
            OpenThatDoor(&door->d2flip);
        }
    }
    else
    {
        if (item->current_anim_state == DOOR_OPEN)
            item->goal_anim_state = DOOR_CLOSED;
        else
        {
            ShutThatDoor(&door->d1);
            ShutThatDoor(&door->d2);
            ShutThatDoor(&door->d1flip);
            ShutThatDoor(&door->d2flip);
        }
    }

    AnimateItem(item);
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

void LiftControl(sint16 item_number)
{
    ITEM_INFO *item;
    LIFT_INFO *lift;
    sint16 room_number;

    item = &items[item_number];
    lift = (LIFT_INFO *)item->data;

    if (TriggerActive(item))
    {
        if (item->pos.y_pos < lift->start_height + STEP_L * 22 - 16)
        {
            if (lift->wait_time < LIFT_WAIT_TIME)
            {
                item->goal_anim_state = DOOR_OPEN;
                lift->wait_time++;
            }
            else
            {
                item->goal_anim_state = DOOR_CLOSED;
                item->pos.y_pos += 16;
            }
        }
        else
        {
            item->goal_anim_state = DOOR_OPEN;
            lift->wait_time = 0;
        }
    }
    else
    {
        if (item->pos.y_pos > lift->start_height + 16)
        {
            if (lift->wait_time < LIFT_WAIT_TIME)
            {
                item->goal_anim_state = DOOR_OPEN;
                lift->wait_time++;
            }
            else
            {
                item->goal_anim_state = DOOR_CLOSED;
                item->pos.y_pos -= 16;
            }
        }
        else
        {
            item->goal_anim_state = DOOR_OPEN;
            lift->wait_time = 0;
        }
    }

    AnimateItem(item);

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
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

void GeneralControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    PHD_VECTOR pos;

    item = &items[item_number];

    if (TriggerActive(item))
        item->goal_anim_state = DOOR_OPEN;
    else
        item->goal_anim_state = DOOR_CLOSED;

    AnimateItem(item);

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    pos.x = 3000;
    pos.y = 720;
    pos.z = 0;
    GetJointAbsPosition(item, &pos, 0);
    AddDynamicLight(pos.x, pos.y, pos.z, 14, 11);

    if (item->status == DEACTIVATED)
    {
        RemoveActiveItem(item_number);
        item->flags |= ONESHOT;
    }
}

void DetonatorControl(sint16 item_number)
{
    int frame_number;
    ITEM_INFO *item;

    item = &items[item_number];

    AnimateItem(item);

    frame_number = item->frame_number - anims[item->anim_number].frame_base;
    if (frame_number > 75 && frame_number < 100)
        AddDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 13, 11);

    if (frame_number == 80)
    {
        camera.bounce = -150;
        SoundEffect(105, NULL, 0);
    }

    if (item->status == DEACTIVATED)
        RemoveActiveItem(item_number);
}
