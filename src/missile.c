#include "game.h"

void ShootAtLara(FX_INFO *fx)
{
    sint32 x, y, z, distance;
    sint16 *bounds;

    x = lara_item->pos.x_pos - fx->pos.x_pos;
    y = lara_item->pos.y_pos - fx->pos.y_pos;
    z = lara_item->pos.z_pos - fx->pos.z_pos;

    bounds = GetBoundsAccurate(lara_item);
    y += bounds[3] + (bounds[2] - bounds[3]) * 3 / 4;

    distance = phd_sqrt(x * x + z * z);
    fx->pos.x_rot = -phd_atan(distance, y);
    fx->pos.y_rot = phd_atan(z, x);

    fx->pos.x_rot += (GetRandomControl() - 0x4000) / 0x40;
    fx->pos.y_rot += (GetRandomControl() - 0x4000) / 0x40;
}
