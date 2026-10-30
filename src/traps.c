#include "game.h"
#include "specific/init.h"
#include "box.h"

#define FALLING_PLANK 57
#define BLADE 81
#define FLAME 252
#define HOOK_DAMAGE 50
#define BLADE_DAMAGE 100
#define BIG_FIRE 0
#define TEETHTRAP_DAMAGE 400

sint16 CreateEffect(sint16 room_num);
int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z);
void KillEffect(sint16 fx_num);
void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite);

extern BITE_INFO teeth1a, teeth1b, teeth2a, teeth2b, teeth3a, teeth3b;

enum trap_anims { TRAP_SET, TRAP_ACTIVATE, TRAP_WORKING, TRAP_FINISHED };
enum door_anims { DOOR_CLOSED, DOOR_OPEN };
enum teeth_anims { TT_NICE, TT_NASTY };
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
