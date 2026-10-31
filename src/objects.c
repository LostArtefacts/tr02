#include <string.h>
#include "game.h"
#include "box.h"
#include "savegame.h"

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
#define COPTER_SPIN 1
#define COPTER_TAKEOFF 2
#define DEATH_SPEED 100
#define DEATH_ACC 5
#define DEATH_GRAB 1
#define EXTRA_DOOR_STUFF 31

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

#define DOG 15
#define CULT1 16
#define CULT3 20
#define WORKER3 32

extern sint16 final_boss[5];
extern sint16 final_level_count;
extern sint16 final_boss_count;
extern int level_items;

void InitialiseFinalLevel(void)
{
    int i;
    ITEM_INFO *item;

    final_level_count = final_boss_count = final_boss_active = 0;

    for (i = 0, item = items; i < level_items; i++, item++)
    {
        switch (item->object_number)
        {
        case CULT1:
        case WORKER3:
        case DOG:
            final_level_count++;
            break;

        case CULT3:
            final_boss[final_boss_count++] = i;
            if (item->status == ACTIVE)
                final_boss_active = 1;
            else if (item->status == DEACTIVATED)
                final_boss_active = 2;
            break;
        }
    }
}

#define EXTRA_FINALANIM 10

extern sint16 cine_frame;
extern PHD_3DPOS cinematic_pos;

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
int EnableBaddieAI(sint16 item_number, int always);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

void FinalLevelCounter(sint16 item_number)
{
    sint16 best;
    ITEM_INFO *item;
    GAME_VECTOR start, target;
    int i, best_distance, distance, x, y, z;

    if (savegame.kills == final_level_count && !final_boss_active)
    {
        best = final_boss[0];
        for (i = 0; i < final_boss_count; i++)
        {
            item = &items[final_boss[i]];

            start.x = lara_item->pos.x_pos;
            start.y = lara_item->pos.y_pos - STEP_L * 2;
            start.z = lara_item->pos.z_pos;
            start.room_number = lara_item->room_number;

            target.x = item->pos.x_pos;
            target.y = item->pos.y_pos - STEP_L * 2;
            target.z = item->pos.z_pos;

            if (!LOS(&start, &target))
            {
                x = (lara_item->pos.x_pos - item->pos.x_pos) >> 6;
                y = (lara_item->pos.y_pos - item->pos.y_pos) >> 6;
                z = (lara_item->pos.z_pos - item->pos.z_pos) >> 6;
                distance = x * x + y * y + z * z;
                if (distance < best_distance)
                {
                    best_distance = distance;
                    best = final_boss[i];
                }
            }
        }

        item = &items[best];
        item->touch_bits = 0;
        item->status = ACTIVE;
        AddActiveItem(best);
        EnableBaddieAI(best, 1);
        item->mesh_bits = 0xffff1fff;
        final_boss_active = 1;
    }
    else if (savegame.kills > final_level_count)
    {
        final_boss_active++;
        if (final_boss_active == FINAL_COUNTDOWN)
        {
            cine_frame = 428;
            item = &items[item_number];
            CreatureKill(item, 0, 0, EXTRA_FINALANIM);
            lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
            camera.type = CINEMATIC_CAMERA;
            memcpy(&cinematic_pos, &item->pos, sizeof(PHD_3DPOS));
        }
    }
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

void SmashIceControl(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    ROOM_INFO *r;

    item = &items[item_number];
    if (item->flags & ONESHOT)
        return;

    item->mesh_bits = 0x0fffe;
    item->collidable = 0;

    r = &room[item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    if (boxes[floor->box].overlap_index & BLOCKED)
        boxes[floor->box].overlap_index -= BLOCKED;

    ExplodingDeath(item_number, 0x0fefe, 0);
    SoundEffect(348, &item->pos, 0);

    item->flags |= ONESHOT;
    item->status = DEACTIVATED;
    RemoveActiveItem(item_number);
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

void InitialiseDoor(sint16 item_number)
{
    int dx, dy;
    ITEM_INFO *item;
    ROOM_INFO *r, *b;
    DOOR_DATA *door;
    sint16 two_room, box_number, room_number;

    item = &items[item_number];
    door = item->data = (DOOR_DATA *)game_malloc(sizeof(DOOR_DATA), EXTRA_DOOR_STUFF);

    dx = dy = 0;
    if (item->pos.y_rot == 0)
        dx--;
    else if (item->pos.y_rot == -0x8000)
        dx++;
    else if (item->pos.y_rot == 0x4000)
        dy--;
    else
        dy++;

    r = &room[item->room_number];
    door->d1.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];
    room_number = GetDoor(door->d1.floor);
    if (room_number == NO_ROOM)
        box_number = door->d1.floor->box;
    else {
        b = &room[room_number];
        box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
    }
    door->d1.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;
    memcpy(&door->d1.data, door->d1.floor, sizeof(FLOOR_INFO));

    if (r->flipped_room != -1) {
        r = &room[r->flipped_room];
        door->d1flip.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];
        room_number = GetDoor(door->d1flip.floor);
        if (room_number == NO_ROOM)
            box_number = door->d1flip.floor->box;
        else {
            b = &room[room_number];
            box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
        }
        door->d1flip.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;
        memcpy(&door->d1flip.data, door->d1flip.floor, sizeof(FLOOR_INFO));
    } else
        door->d1flip.floor = NULL;

    two_room = GetDoor(door->d1.floor);

    ShutThatDoor(&door->d1);
    ShutThatDoor(&door->d1flip);

    if (two_room == NO_ROOM) {
        door->d2.floor = NULL;
        door->d2flip.floor = NULL;
    } else {
        r = &room[two_room];
        door->d2.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
        room_number = GetDoor(door->d2.floor);
        if (room_number == NO_ROOM)
            box_number = door->d2.floor->box;
        else {
            b = &room[room_number];
            box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
        }
        door->d2.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;
        memcpy(&door->d2.data, door->d2.floor, sizeof(FLOOR_INFO));

        if (r->flipped_room != -1) {
            r = &room[r->flipped_room];
            door->d2flip.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
            room_number = GetDoor(door->d2flip.floor);
            if (room_number == NO_ROOM)
                box_number = door->d2flip.floor->box;
            else {
                b = &room[room_number];
                box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
            }
            door->d2flip.block = (boxes[box_number].overlap_index & BLOCKABLE) ? box_number : NO_BOX;
            memcpy(&door->d2flip.data, door->d2flip.floor, sizeof(FLOOR_INFO));
        } else
            door->d2flip.floor = NULL;

        ShutThatDoor(&door->d2);
        ShutThatDoor(&door->d2flip);

        room_number = item->room_number;
        ItemNewRoom(item_number, two_room);
        item->room_number = room_number;
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

#define LIFT_HEIGHT (STEP_L * 5)

void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling)
{
    int ix, iz, lx, lz, point_in_shaft;
    int lift_floor, lift_ceiling;

    ix = item->pos.x_pos >> WALL_SHIFT;
    iz = item->pos.z_pos >> WALL_SHIFT;

    lx = lara_item->pos.x_pos >> WALL_SHIFT;
    lz = lara_item->pos.z_pos >> WALL_SHIFT;

    x >>= WALL_SHIFT;
    z >>= WALL_SHIFT;

    point_in_shaft = ((x == ix || x + 1 == ix) && (z == iz || z - 1 == iz));

    lift_floor = item->pos.y_pos;
    lift_ceiling = item->pos.y_pos - LIFT_HEIGHT;

    *floor = 0x7fff;
    *ceiling = -0x7fff;

    if ((lx == ix || lx + 1 == ix) && (lz == iz || lz - 1 == iz))
    {
        if (item->current_anim_state == DOOR_CLOSED && lara_item->pos.y_pos < lift_floor + STEP_L && lara_item->pos.y_pos > lift_ceiling + STEP_L)
        {
            if (point_in_shaft)
            {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            }
            else
            {
                *floor = NO_HEIGHT;
                *ceiling = 0x7fff;
            }
        }
        else if (point_in_shaft)
        {
            if (lara_item->pos.y_pos < lift_ceiling + STEP_L)
                *floor = lift_ceiling;
            else if (lara_item->pos.y_pos < lift_floor + STEP_L)
            {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            }
            else
                *ceiling = lift_floor + STEP_L;
        }
    }
    else if (point_in_shaft)
    {
        if (y <= lift_ceiling)
            *floor = lift_ceiling;
        else if (y < lift_floor + STEP_L)
        {
            if (item->current_anim_state == DOOR_CLOSED)
            {
                *floor = NO_HEIGHT;
                *ceiling = 0x7fff;
            }
            else
            {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            }
        }
        else
            *ceiling = lift_floor + STEP_L;
    }
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

void CopterControl(sint16 item_number)
{
    ITEM_INFO *item;
    PHD_VECTOR pos;
    sint16 room_number, *bounds;

    item = &items[item_number];

    if (item->current_anim_state == COPTER_SPIN)
    {
        if (item->flags & ONESHOT)
            item->goal_anim_state = COPTER_TAKEOFF;
    }

    AnimateItem(item);

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    bounds = GetBoundsAccurate(item);
    pos.x = (bounds[0] + bounds[1]) >> 1;
    pos.y = (bounds[2] + bounds[3]) >> 1;
    pos.z = (bounds[4] + bounds[5]) >> 1;
    pos.x = lara_item->pos.x_pos + ((pos.x - lara_item->pos.x_pos) >> 2);
    pos.y = lara_item->pos.y_pos + ((pos.y - lara_item->pos.y_pos) >> 2);
    pos.z = lara_item->pos.z_pos + ((pos.z - lara_item->pos.z_pos) >> 2);
    SoundEffect(297, (PHD_3DPOS *)&pos, 0);

    if (item->status == DEACTIVATED)
        KillItem(item_number);
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
