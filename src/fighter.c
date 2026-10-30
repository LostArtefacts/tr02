#include "game.h"
#include "box.h"

void ShootAtLara(FX_INFO *fx);

#define KNIFE 247
#define KNIFE_SPEED 150

sint16 Knife(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    sint16 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->room_number = room_number;
        fx->pos.x_rot = fx->pos.z_rot = 0;
        fx->pos.y_rot = yrot;
        fx->speed = KNIFE_SPEED;
        fx->frame_number = 0;
        fx->object_number = KNIFE;
        fx->shade = 14 * 256;
        ShootAtLara(fx);
    }
    return fx_number;
}
