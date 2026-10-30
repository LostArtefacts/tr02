#include "game.h"
#include "specific/init.h"
#include "box.h"

#define FALLING_PLANK 57
#define BLADE 81
#define FLAME 252
#define RICOCHET 238
#define FLAME_ONFIRE_DAMAGE 7
#define FLAME_TOONEAR_DAMAGE 5
#define HOOK_DAMAGE 50
#define SPIKEWALL_DAMAGE 20
#define PROPELLER_DAMAGE 200
#define SPINBLADE_DAMAGE 100
#define SPIKE_DAMAGE 15
#define SPIKE_DEATH_ANIM 149
#define DARTS 61
#define ROLLINGBALL_DAMAGE_AIR 100
#define ROLLINGBALL_DEATH_ANIM 139
#define LAVA 250
#define LAVA_GLOB_DAMAGE 10
#define GRAVITY 6
#define PROPELLER1 76
#define PROPELLER2 77
#define PROPELLER3 94
#define BLADE_DAMAGE 100
#define BIG_FIRE 0
#define TEETHTRAP_DAMAGE 400
#define BOARD_OFF 0
#define FALLING_CEILING_DAMAGE 300
#define STATUE_DAMAGE 20
#define ICICLE_DAMAGE 200
#define ICICLE_BREAK 1
#define ICICLE_FALL 2
#define ICICLE_LAND 3
#define STATUE_TOUCH 0x80
#define BOARD_ON 1
#define FALLDOWN_A 34
#define FALLDOWN_F GF(FALLDOWN_A, 0)

sint16 CreateEffect(sint16 room_num);
int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 distance);
void LaraBurn(void);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite);

extern BITE_INFO teeth1a, teeth1b, teeth2a, teeth2b, teeth3a, teeth3b;

enum trap_anims { TRAP_SET, TRAP_ACTIVATE, TRAP_WORKING, TRAP_FINISHED };
enum door_anims { DOOR_CLOSED, DOOR_OPEN };
enum teeth_anims { TT_NICE, TT_NASTY };
enum statue_anims { STATUE_EMPTY, STATUE_STOP, STATUE_CUT };
enum blade_anims { BLADE_EMPTY, BLADE_STOP, BLADE_CUT };
enum prop_anims { PROP_ON, PROP_OFF };
enum spin_anims { SPIN_EMPTY, SPIN_STOP, SPIN_SPIN };
enum dart_anims { DART_OFF, DART_FIRE };

static int clonked = 0;

void ControlSpikeWall(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 x, z;
    sint16 room_number;

    item = &items[item_number];

    if (TriggerActive(item) && item->status != DEACTIVATED)
    {
        z = item->pos.z_pos + (phd_cos(item->pos.y_rot) >> 10);
        x = item->pos.x_pos + (phd_sin(item->pos.y_rot) >> 10);

        room_number = item->room_number;
        floor = GetFloor(x, item->pos.y_pos, z, &room_number);
        if (GetHeight(floor, x, item->pos.y_pos, z) != item->pos.y_pos)
            item->status = DEACTIVATED;
        else
        {
            item->pos.z_pos = z;
            item->pos.x_pos = x;
            if (room_number != item->room_number)
                ItemNewRoom(item_number, room_number);
        }

        SoundEffect(204, &item->pos, 0);
    }

    if (item->touch_bits)
    {
        lara_item->hit_points -= SPIKEWALL_DAMAGE;
        lara_item->hit_status = 1;
        DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - WALL_L / 2, lara_item->pos.z_pos, 1, item->pos.y_rot, lara_item->room_number, 3);
        item->touch_bits = 0;
        SoundEffect(205, &item->pos, 0);
    }
}

void ControlCeilingSpikes(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 y;
    sint16 room_number;

    item = &items[item_number];

    if (TriggerActive(item) && item->status != DEACTIVATED)
    {
        y = item->pos.y_pos + 5;

        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
        if (GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos) < y + WALL_L)
            item->status = DEACTIVATED;
        else
        {
            item->pos.y_pos = y;
            if (room_number != item->room_number)
                ItemNewRoom(item_number, room_number);
        }

        SoundEffect(204, &item->pos, 0);
    }

    if (item->touch_bits)
    {
        lara_item->hit_points -= SPIKEWALL_DAMAGE;
        lara_item->hit_status = 1;
        DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos + 768, lara_item->pos.z_pos, 1, item->pos.y_rot, lara_item->room_number, 3);
        item->touch_bits = 0;
        SoundEffect(205, &item->pos, 0);
    }
}

void HookControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (item->touch_bits && !clonked)
    {
        lara_item->hit_points -= HOOK_DAMAGE;
        lara_item->hit_status = 1;

        DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - (WALL_L / 2), lara_item->pos.z_pos,
            lara_item->speed, lara_item->pos.y_rot, lara_item->room_number, 3);
    }
    else
        clonked = 0;

    AnimateItem(item);
}

void PropellerControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (TriggerActive(item) && !(item->flags & ONESHOT))
    {
        item->goal_anim_state = PROP_ON;

        if (item->touch_bits & 6)
        {
            lara_item->hit_points -= PROPELLER_DAMAGE;
            lara_item->hit_status = 1;
            DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - WALL_L / 2, lara_item->pos.z_pos, (sint16)(GetRandomDraw() >> 10), item->pos.y_rot + 0x4000, lara_item->room_number, 3);
            if (item->object_number == PROPELLER2)
                SoundEffect(207, &item->pos, 0);
        }
        else
        {
            if (item->object_number == PROPELLER2)
                SoundEffect(206, &item->pos, 0);
            else if (item->object_number == PROPELLER1)
                SoundEffect(213, &item->pos, 0);
            else if (item->object_number == PROPELLER3)
                SoundEffect(215, &item->pos, 1);
            else
                SoundEffect(217, &item->pos, 0);
        }
    }
    else if (item->goal_anim_state != PROP_OFF)
    {
        if (item->object_number == PROPELLER1)
            SoundEffect(213, &item->pos, 0);
        else if (item->object_number == PROPELLER3)
            SoundEffect(216, &item->pos, 1);
        item->goal_anim_state = PROP_OFF;
    }

    AnimateItem(item);

    if (item->status == DEACTIVATED)
    {
        RemoveActiveItem(item_number);
        if (item->object_number != PROPELLER2)
            item->collidable = 0;
    }
}

void SpinningBlade(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 x, z;
    sint16 room_number;
    int spinning;

    item = &items[item_number];

    if (item->current_anim_state == SPIN_SPIN)
    {
        if (item->goal_anim_state != SPIN_STOP)
        {
            z = item->pos.z_pos + ((WALL_L * 3 / 2 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
            x = item->pos.x_pos + ((WALL_L * 3 / 2 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
            room_number = item->room_number;
            floor = GetFloor(x, item->pos.y_pos, z, &room_number);
            if (GetHeight(floor, x, item->pos.y_pos, z) == NO_HEIGHT)
                item->goal_anim_state = SPIN_STOP;
        }

        spinning = 1;

        if (item->touch_bits)
        {
            lara_item->hit_status = 1;
            lara_item->hit_points -= SPINBLADE_DAMAGE;
            DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - WALL_L / 2, lara_item->pos.z_pos, (sint16)(item->speed * 2), lara_item->pos.y_rot, lara_item->room_number, 2);
        }

        SoundEffect(231, &item->pos, 0);
    }
    else
    {
        if (TriggerActive(item))
            item->goal_anim_state = SPIN_SPIN;
        spinning = 0;
    }

    AnimateItem(item);

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    if (spinning && item->current_anim_state == SPIN_STOP)
        item->pos.y_rot += -0x8000;
}

void IcicleControl(sint16 item_number)
{
    sint16 room_number;
    ITEM_INFO *item;
    FLOOR_INFO *floor;

    item = &items[item_number];

    switch (item->current_anim_state)
    {
    case ICICLE_BREAK:
        item->goal_anim_state = ICICLE_FALL;
        break;

    case ICICLE_FALL:
        if (item->gravity_status == 0)
        {
            item->fallspeed = 50;
            item->gravity_status = 1;
        }
        if (item->touch_bits)
        {
            lara_item->hit_points -= ICICLE_DAMAGE;
            lara_item->hit_status = 1;
        }
        break;

    case ICICLE_LAND:
        item->gravity_status = 0;
        break;
    }

    AnimateItem(item);
    if (item->status == DEACTIVATED)
    {
        RemoveActiveItem(item_number);
        return;
    }

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->current_anim_state == ICICLE_FALL && item->pos.y_pos >= item->floor)
    {
        item->gravity_status = 0;
        item->goal_anim_state = ICICLE_LAND;
        item->pos.y_pos = item->floor;
        item->fallspeed = 0;
        item->mesh_bits = 0x2b;
    }
}

void InitialiseBlade(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->anim_number = objects[BLADE].anim_index + 2;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = BLADE_STOP;
}

void BladeControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (TriggerActive(item) && item->current_anim_state == BLADE_STOP)
        item->goal_anim_state = BLADE_CUT;
    else
        item->goal_anim_state = BLADE_STOP;

    if ((item->touch_bits & 0x2) && item->current_anim_state == BLADE_CUT)
    {
        lara_item->hit_status = 1;
        lara_item->hit_points -= BLADE_DAMAGE;

        DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos - STEP_L, lara_item->pos.z_pos,
            lara_item->speed, lara_item->pos.y_rot, lara_item->room_number, 2);
    }

    AnimateItem(item);
}

void InitialiseKillerStatue(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->anim_number = objects[item->object_number].anim_index + 3;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = BLADE_STOP;
}

void KillerStatueControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 x, y, z;
    sint16 d;

    item = &items[item_number];

    if (TriggerActive(item) && item->current_anim_state == STATUE_STOP)
        item->goal_anim_state = STATUE_CUT;
    else
        item->goal_anim_state = STATUE_STOP;

    if ((item->touch_bits & STATUE_TOUCH) && item->current_anim_state == STATUE_CUT)
    {
        lara_item->hit_status = 1;
        lara_item->hit_points -= STATUE_DAMAGE;

        x = lara_item->pos.x_pos + (GetRandomControl() - 16384) / 256;
        z = lara_item->pos.z_pos + (GetRandomControl() - 16384) / 256;
        y = lara_item->pos.y_pos - GetRandomControl() / 44;
        d = (GetRandomControl() - 16384) / 8 + lara_item->pos.y_rot;
        DoBloodSplat(x, y, z, lara_item->speed, d, lara_item->room_number);
    }

    AnimateItem(item);
}

void SpringBoardControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (item->current_anim_state == BOARD_OFF && lara_item->pos.y_pos == item->pos.y_pos
        && (lara_item->pos.x_pos >> WALL_SHIFT) == (item->pos.x_pos >> WALL_SHIFT)
        && (lara_item->pos.z_pos >> WALL_SHIFT) == (item->pos.z_pos >> WALL_SHIFT))
    {
        if (lara_item->hit_points <= 0)
            return;

        if (lara_item->current_anim_state == AS_BACK || lara_item->current_anim_state == AS_FASTBACK)
            lara_item->speed = -lara_item->speed;

        lara_item->fallspeed = -240;
        lara_item->gravity_status = 1;

        lara_item->anim_number = FALLDOWN_A;
        lara_item->frame_number = FALLDOWN_F;
        lara_item->current_anim_state = AS_FORWARDJUMP;
        lara_item->goal_anim_state = AS_FORWARDJUMP;

        item->goal_anim_state = BOARD_ON;
    }

    AnimateItem(item);
}

void InitialiseRollingBall(sint16 item_number)
{
    ITEM_INFO *item;
    GAME_VECTOR *old;

    item = &items[item_number];

    old = item->data = (GAME_VECTOR *)game_malloc(sizeof(GAME_VECTOR), ROLLINGBALL_STUFF);
    old->x = item->pos.x_pos;
    old->y = item->pos.y_pos;
    old->z = item->pos.z_pos;
    old->room_number = item->room_number;
}

void RollingBallCollision(sint16 item_num, ITEM_INFO *litem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    sint32 x, y, z, i;
    sint16 d;

    item = &items[item_num];

    if (item->status == ACTIVE)
    {
        if (!TestBoundsCollide(item, litem, coll->radius))
            return;
        if (!TestCollision(item, litem))
            return;

        if (litem->gravity_status)
        {
            if (coll->enable_baddie_push)
                ItemPushLara(item, litem, coll, coll->enable_spaz, 1);
            litem->hit_points -= ROLLINGBALL_DAMAGE_AIR;
            x = litem->pos.x_pos - item->pos.x_pos;
            y = litem->pos.y_pos - item->pos.y_pos + 162;
            z = litem->pos.z_pos - item->pos.z_pos;
            d = (sint16)phd_sqrt(x * x + y * y + z * z);
            if (d < WALL_L / 2)
                d = WALL_L / 2;
            x = item->pos.x_pos + (x << WALL_SHIFT) / 2 / d;
            y = item->pos.y_pos + (y << WALL_SHIFT) / 2 / d - WALL_L / 2;
            z = item->pos.z_pos + (z << WALL_SHIFT) / 2 / d;
            DoBloodSplat(x, y, z, item->speed, item->pos.y_rot, item->room_number);
        }
        else
        {
            litem->hit_status = 1;
            if (litem->hit_points > 0)
            {
                litem->hit_points = -1;
                litem->pos.y_rot = item->pos.y_rot;
                litem->pos.z_rot = 0;
                litem->pos.x_rot = 0;
                litem->anim_number = ROLLINGBALL_DEATH_ANIM;
                litem->frame_number = anims[ROLLINGBALL_DEATH_ANIM].frame_base;
                litem->current_anim_state = AS_SPECIAL;
                litem->goal_anim_state = AS_SPECIAL;

                camera.flags = FOLLOW_CENTRE;
                camera.target_angle = 30940;
                camera.target_elevation = -4550;

                for (i = 0; i < 15; i++)
                {
                    x = litem->pos.x_pos + (GetRandomControl() - 0x4000) / 256;
                    z = litem->pos.z_pos + (GetRandomControl() - 0x4000) / 256;
                    y = litem->pos.y_pos - GetRandomControl() / 64;
                    d = item->pos.y_rot + (GetRandomControl() - 0x4000) / 8;
                    DoBloodSplat(x, y, z, (sint16)(item->speed * 2), d, item->room_number);
                }
            }
        }
    }
    else if (item->status != INVISIBLE)
        ObjectCollision(item_num, litem, coll);
}

void SpikeCollision(sint16 item_num, ITEM_INFO *litem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    int num, x, y, z;

    item = &items[item_num];

    if (litem->hit_points < 0)
        return;
    if (!TestBoundsCollide(item, litem, coll->radius))
        return;
    if (!TestCollision(item, litem))
        return;

    num = GetRandomControl() / 24576;
    if (litem->gravity_status)
    {
        if (litem->fallspeed > 6)
        {
            litem->hit_points = -1;
            num = 20;
        }
    }
    else if (litem->speed < 30)
        return;

    litem->hit_points -= SPIKE_DAMAGE;
    for (; num > 0; num--)
    {
        x = litem->pos.x_pos + (GetRandomControl() - 0x4000) / 256;
        z = litem->pos.z_pos + (GetRandomControl() - 0x4000) / 256;
        y = litem->pos.y_pos - GetRandomControl() / 64;
        DoBloodSplat(x, y, z, 20, GetRandomControl(), item->room_number);
    }

    if (litem->hit_points <= 0)
    {
        litem->anim_number = SPIKE_DEATH_ANIM;
        litem->frame_number = anims[SPIKE_DEATH_ANIM].frame_base;
        litem->current_anim_state = AS_DEATH;
        litem->goal_anim_state = AS_DEATH;
        litem->pos.y_pos = item->pos.y_pos;
        litem->gravity_status = 0;
    }
}

void TrapDoorControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (TriggerActive(item))
    {
        if (item->current_anim_state == DOOR_CLOSED)
            item->goal_anim_state = DOOR_OPEN;
    }
    else
    {
        if (item->current_anim_state == DOOR_OPEN)
            item->goal_anim_state = DOOR_CLOSED;
    }

    AnimateItem(item);
}

void TrapDoorFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (!OnTrapDoor(item, x, z))
        return;
    if (y <= item->pos.y_pos)
    {
        if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos < *height)
            *height = item->pos.y_pos;
    }
}

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

void Pendulum(sint16 item_number)
{
    int x, y, z;
    sint16 d;
    ITEM_INFO *item;
    FLOOR_INFO *floor;

    item = &items[item_number];

    if (item->touch_bits)
    {
        lara_item->hit_points -= 50;
        lara_item->hit_status = 1;
        x = lara_item->pos.x_pos + (GetRandomControl() - 16384) / 256;
        z = lara_item->pos.z_pos + (GetRandomControl() - 16384) / 256;
        y = lara_item->pos.y_pos - GetRandomControl() / 44;
        d = (GetRandomControl() - 16384) / 8 + lara_item->pos.y_rot;
        DoBloodSplat(x, y, z, lara_item->speed, d, lara_item->room_number);
    }

    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &item->room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    AnimateItem(item);
}

void FallingBlock(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 origin;

    item = &items[item_number];

    if (item->object_number == FALLING_PLANK)
        origin = WALL_L;
    else
        origin = STEP_L * 2;

    switch (item->current_anim_state)
    {
    case TRAP_SET:
        if (lara_item->pos.y_pos == item->pos.y_pos - origin)
            item->goal_anim_state = TRAP_ACTIVATE;
        else
        {
            item->status = NOT_ACTIVE;
            RemoveActiveItem(item_number);
            return;
        }
        break;

    case TRAP_ACTIVATE:
        item->goal_anim_state = TRAP_WORKING;
        break;

    case TRAP_WORKING:
        if (item->goal_anim_state != TRAP_FINISHED)
            item->gravity_status = 1;
        break;
    }

    AnimateItem(item);
    if (item->status == DEACTIVATED)
    {
        RemoveActiveItem(item_number);
        return;
    }

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->current_anim_state == TRAP_WORKING && item->pos.y_pos >= item->floor)
    {
        item->gravity_status = 0;
        item->goal_anim_state = TRAP_FINISHED;
        item->pos.y_pos = item->floor;
        item->fallspeed = 0;
    }
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

void TeethTrap(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (TriggerActive(item))
    {
        item->goal_anim_state = TT_NASTY;
        if (item->touch_bits && item->current_anim_state == TT_NASTY)
        {
            lara_item->hit_points -= TEETHTRAP_DAMAGE;
            lara_item->hit_status = 1;
            BaddieBiteEffect(item, &teeth1a);
            BaddieBiteEffect(item, &teeth1b);
            BaddieBiteEffect(item, &teeth2a);
            BaddieBiteEffect(item, &teeth2b);
            BaddieBiteEffect(item, &teeth3a);
            BaddieBiteEffect(item, &teeth3b);
        }
    }
    else
        item->goal_anim_state = TT_NICE;

    AnimateItem(item);
}

void DartEmitterControl(sint16 item_num)
{
    ITEM_INFO *item, *dart;
    sint16 dart_item_num;
    sint32 xoff, zoff;

    item = &items[item_num];

    if (TriggerActive(item))
    {
        if (item->current_anim_state == DART_OFF)
            item->goal_anim_state = DART_FIRE;
    }
    else
    {
        if (item->current_anim_state == DART_FIRE)
            item->goal_anim_state = DART_OFF;
    }

    if (item->current_anim_state == DART_FIRE && item->frame_number == anims[item->anim_number].frame_base)
    {
        dart_item_num = CreateItem();
        if (dart_item_num != NO_ITEM)
        {
            dart = &items[dart_item_num];
            dart->object_number = DARTS;
            dart->room_number = item->room_number;
            dart->shade = -1;
            dart->pos.y_rot = item->pos.y_rot;
            dart->pos.y_pos = item->pos.y_pos - WALL_L / 2;

            xoff = 0;
            zoff = 0;
            switch (dart->pos.y_rot)
            {
            case 0:
                zoff = -412;
                break;
            case 16364:
                xoff = -412;
                break;
            case -0x8000:
                zoff = 412;
                break;
            case -0x4000:
                xoff = 412;
                break;
            }

            dart->pos.x_pos = item->pos.x_pos + xoff;
            dart->pos.z_pos = item->pos.z_pos + zoff;
            InitialiseItem(dart_item_num);
            AddActiveItem(dart_item_num);
            dart->status = ACTIVE;
            SoundEffect(254, &dart->pos, 0);
        }
    }

    AnimateItem(item);
}

void DartsControl(sint16 item_num)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    FX_INFO *fx;
    sint16 room_number, fx_num;

    item = &items[item_num];

    if (item->touch_bits)
    {
        lara_item->hit_points -= 50;
        lara_item->hit_status = 1;
        DoBloodSplat(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
    }

    AnimateItem(item);

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_num, room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    item->pos.x_rot += 0x1000;

    if (item->pos.y_pos >= item->floor)
    {
        KillItem(item_num);
        fx_num = CreateEffect(item->room_number);
        if (fx_num != NO_ITEM)
        {
            fx = &effects[fx_num];
            fx->pos = item->pos;
            fx->speed = 0;
            fx->frame_number = -3 * GetRandomControl() / 0x8000;
            fx->counter = 6;
            fx->object_number = RICOCHET;
        }
        SoundEffect(258, &item->pos, 0);
    }
}

void DartEffectControl(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->counter++;
    if (fx->counter >= 3)
    {
        fx->counter = 0;
        fx->frame_number--;
        if (fx->frame_number <= objects[fx->object_number].nmeshes)
            KillEffect(fx_num);
    }
}

void FallingCeiling(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_number];

    if (item->current_anim_state == TRAP_SET)
    {
        item->goal_anim_state = TRAP_ACTIVATE;
        item->gravity_status = 1;
    }
    else if (item->current_anim_state == TRAP_ACTIVATE)
    {
        if (item->touch_bits)
        {
            lara_item->hit_points -= FALLING_CEILING_DAMAGE;
            lara_item->hit_status = 1;
        }
    }

    AnimateItem(item);
    if (item->status == DEACTIVATED)
    {
        RemoveActiveItem(item_number);
        return;
    }

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    if (item->current_anim_state == TRAP_ACTIVATE && item->pos.y_pos >= item->floor)
    {
        item->goal_anim_state = TRAP_WORKING;
        item->pos.y_pos = item->floor;
        item->fallspeed = 0;
        item->gravity_status = 0;
    }
}

void FlameEmitterControl(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];

    if (TriggerActive(item))
    {
        if (item->data == NULL)
        {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM)
            {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos;
                fx->pos.y_pos = item->pos.y_pos;
                fx->pos.z_pos = item->pos.z_pos;
                fx->frame_number = BIG_FIRE;
                fx->object_number = FLAME;
                fx->counter = 0;
            }

            item->data = (void *)(fx_number + 1);
        }
    }
    else if (item->data)
    {
        KillEffect((sint16)((int)(item->data) - 1));
        item->data = NULL;
    }
}

void FlameControl(sint16 fx_number)
{
    FX_INFO *fx;
    sint32 x, y, z;

    fx = &effects[fx_number];

    if (--fx->frame_number <= objects[FLAME].nmeshes)
        fx->frame_number = 0;

    if (fx->counter < 0)
    {
        fx->pos.x_pos = 0;
        fx->pos.y_pos = 0;
        if (fx->counter == -1)
            fx->pos.z_pos = -100;
        else
            fx->pos.z_pos = 0;
        GetJointAbsPosition(lara_item, (PHD_VECTOR *)&fx->pos, -1 - fx->counter);
        if (lara_item->room_number != fx->room_number)
            EffectNewRoom(fx_number, lara_item->room_number);

        y = GetWaterHeight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number);
        if (y != NO_HEIGHT && fx->pos.y_pos > y)
        {
            fx->counter = 0;
            KillEffect(fx_number);
            lara.burn = 0;
        }
        else
        {
            SoundEffect(150, &fx->pos, 0);
            lara_item->hit_points -= FLAME_ONFIRE_DAMAGE;
            lara_item->hit_status = 1;
        }
        return;
    }

    SoundEffect(150, &fx->pos, 0);
    if (fx->counter)
        fx->counter--;
    else if (ItemNearLara(&fx->pos, 600))
    {
        x = lara_item->pos.x_pos - fx->pos.x_pos;
        z = lara_item->pos.z_pos - fx->pos.z_pos;
        lara_item->hit_points -= FLAME_TOONEAR_DAMAGE;
        lara_item->hit_status = 1;
        if (x * x + z * z < 450 * 450)
        {
            fx->counter = 100;
            LaraBurn();
        }
    }
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

void LavaBurn(ITEM_INFO *item)
{
    int i;
    FX_INFO *fx;
    sint16 fx_number, room_number;
    FLOOR_INFO *floor;

    if (item->hit_points < 0)
        return;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, 32000, item->pos.z_pos, &room_number);
    if (item->floor != GetHeight(floor, item->pos.x_pos, 32000, item->pos.z_pos))
        return;

    item->hit_status = 1;
    item->hit_points = -1;
    for (i = 0; i < 10; i++)
    {
        fx_number = CreateEffect(item->room_number);
        if (fx_number != NO_ITEM)
        {
            fx = &effects[fx_number];
            fx->frame_number = (objects[FLAME].nmeshes * GetRandomControl()) / 0x7fff;
            fx->object_number = FLAME;
            fx->counter = -1 - GetRandomControl() * 24 / 0x7fff;
        }
    }
}
