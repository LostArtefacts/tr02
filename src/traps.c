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
void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite);

extern BITE_INFO teeth1a, teeth1b, teeth2a, teeth2b, teeth3a, teeth3b;

enum trap_anims { TRAP_SET, TRAP_ACTIVATE, TRAP_WORKING, TRAP_FINISHED };
enum door_anims { DOOR_CLOSED, DOOR_OPEN };
enum teeth_anims { TT_NICE, TT_NASTY };
enum statue_anims { STATUE_EMPTY, STATUE_STOP, STATUE_CUT };
enum blade_anims { BLADE_EMPTY, BLADE_STOP, BLADE_CUT };

static int clonked = 0;

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
