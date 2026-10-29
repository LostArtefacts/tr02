#include "game.h"

#define MAX_BOUNCE 100

void finish_level_effect(ITEM_INFO *item)
{
    level_complete = 1;
}

void turn180_effect(ITEM_INFO *item)
{
    item->pos.y_rot += -0x8000;
    item->pos.x_rot = -item->pos.x_rot;
}

void floor_shake_effect(ITEM_INFO *item)
{
    int x, y, z, dist;

    x = item->pos.x_pos - camera.pos.x;
    y = item->pos.y_pos - camera.pos.y;
    z = item->pos.z_pos - camera.pos.z;
    if (ABS(x) < 16 * WALL_L && ABS(y) < 16 * WALL_L && ABS(z) < 16 * WALL_L)
    {
        dist = (x * x + y * y + z * z) / 256;
        camera.bounce = ((WALL_L * WALL_L - dist) * MAX_BOUNCE) / (WALL_L * WALL_L);
    }
}

void lara_hands_free(ITEM_INFO *item)
{
    lara.gun_status = LG_ARMLESS;
}
