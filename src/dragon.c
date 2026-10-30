#include "game.h"

#define DRAGON_FRONT 22
#define TWINKLE 239

void KillEffect(sint16 fx_num);

void ControlTwinkle(sint16 fx_number)
{
    int c, s;
    FX_INFO *fx;
    ITEM_INFO *item;
    PHD_VECTOR pos;

    fx = &effects[fx_number];

    fx->frame_number--;
    if (fx->frame_number <= objects[fx->object_number].nmeshes)
        fx->frame_number = 0;

    if (fx->counter >= 0)
    {
        item = &items[fx->counter];

        pos.x = item->pos.x_pos;
        pos.y = item->pos.y_pos;
        pos.z = item->pos.z_pos;
        if (item->object_number == DRAGON_FRONT)
        {
            c = phd_cos(item->pos.y_rot);
            s = phd_sin(item->pos.y_rot);
            pos.z += (490 * c - 1100 * s) >> W2V_SHIFT;
            pos.x += (490 * s + 1100 * c) >> W2V_SHIFT;
            pos.y -= 540;
        }
        fx->pos.x_pos += (pos.x - fx->pos.x_pos) >> 4;
        fx->pos.y_pos += (pos.y - fx->pos.y_pos) >> 4;
        fx->pos.z_pos += (pos.z - fx->pos.z_pos) >> 4;

        if (ABS(fx->pos.x_pos - pos.x) < STEP_L && ABS(fx->pos.y_pos - pos.y) < STEP_L && ABS(fx->pos.z_pos - pos.z) < STEP_L)
            KillEffect(fx_number);
    }
    else
    {
        fx->counter++;
        if (fx->counter == 0)
            KillEffect(fx_number);
    }
}

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

void CreateBartoliLight(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    int fx_number;

    item = &items[item_number];
    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->object_number = TWINKLE;
        fx->pos.y_rot = (sint16)(GetRandomDraw() << 1);
        fx->pos.x_pos = item->pos.x_pos + (WALL_L * 5 * phd_sin(fx->pos.y_rot) >> W2V_SHIFT);
        fx->pos.y_pos = item->pos.y_pos + (GetRandomDraw() >> 2) - WALL_L;
        fx->pos.z_pos = item->pos.z_pos + (WALL_L * 5 * phd_cos(fx->pos.y_rot) >> W2V_SHIFT);
        fx->room_number = item->room_number;
        fx->counter = item_number;
        fx->frame_number = 0;
    }

    AddDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (GetRandomDraw() * 4 >> 15) + 12, (GetRandomDraw() * 4 >> 15) + 10);
}

