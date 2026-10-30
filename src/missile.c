#include "game.h"

#define BLOOD1 233
#define RICOCHET1 238
#define DIVER_HARPOON 245
#define DRAGON_FIRE 246
#define KNIFE 247
#define UNDERWATER 1

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number);
void LaraBurn(void);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
int ItemNearLara(PHD_3DPOS *pos, sint32 distance);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
void KillEffect(sint16 fx_num);

void ControlMissile(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 speed;

    fx = &effects[fx_number];
    if (fx->object_number == DIVER_HARPOON && !(room[fx->room_number].flags & UNDERWATER) && fx->pos.x_rot > -0x3000)
        fx->pos.x_rot -= ONE_DEGREE;
    fx->pos.y_pos += fx->speed * phd_sin(-fx->pos.x_rot) >> W2V_SHIFT;
    speed = fx->speed * phd_cos(fx->pos.x_rot) >> W2V_SHIFT;
    fx->pos.z_pos += speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT;
    fx->pos.x_pos += speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT;
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)
        || fx->pos.y_pos <= GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
    {
        if (fx->object_number == KNIFE || fx->object_number == DIVER_HARPOON)
        {
            fx->speed = 0;
            fx->frame_number = -GetRandomControl() / 11000;
            fx->counter = 6;
            fx->object_number = RICOCHET1;
            SoundEffect(258, &fx->pos, 0);
        }
        else if (fx->object_number == DRAGON_FIRE)
        {
            AddDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 14, 11);
            KillEffect(fx_number);
        }
        return;
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
    if (fx->object_number == DRAGON_FIRE)
    {
        if (ItemNearLara(&fx->pos, 350))
        {
            lara_item->hit_points -= 3;
            lara_item->hit_status = 1;
            LaraBurn();
            return;
        }
    }
    else if (ItemNearLara(&fx->pos, 200))
    {
        if (fx->object_number == KNIFE || fx->object_number == DIVER_HARPOON)
        {
            lara_item->hit_points -= 50;
            fx->object_number = BLOOD1;
            SoundEffect(317, &fx->pos, 0);
        }
        lara_item->hit_status = 1;
        fx->pos.y_rot = lara_item->pos.y_rot;
        fx->speed = lara_item->speed;
        fx->frame_number = fx->counter = 0;
    }
    if (fx->object_number == DIVER_HARPOON && (room[fx->room_number].flags & UNDERWATER))
        CreateBubble(&fx->pos, fx->room_number);
    else if (fx->object_number == DRAGON_FIRE && !fx->counter--)
    {
        AddDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 14, 11);
        SoundEffect(305, &fx->pos, 0);
        KillEffect(fx_number);
    }
    else if (fx->object_number == KNIFE)
        fx->pos.z_rot += 30 * ONE_DEGREE;
}

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

#define GRAVITY 6
#define SPLASH1 230
#define EXPLOSION1 229


void ControlBodyPart(sint16 fx_number)
{
    FX_INFO *fx, *splash;
    FLOOR_INFO *floor;
    sint32 height, ceiling;
    sint16 room_number, new_fx;

    fx = &effects[fx_number];
    fx->pos.x_rot += ONE_DEGREE * 5;
    fx->pos.z_rot += ONE_DEGREE * 10;
    fx->pos.z_pos += fx->speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT;
    fx->pos.x_pos += fx->speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT;
    fx->fallspeed += GRAVITY;
    fx->pos.y_pos += fx->fallspeed;

    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

    if (!(room[fx->room_number].flags & UNDERWATER) && (room[room_number].flags & UNDERWATER))
    {
        new_fx = CreateEffect(fx->room_number);
        if (new_fx != NO_ITEM)
        {
            splash = &effects[new_fx];
            splash->pos.x_pos = fx->pos.x_pos;
            splash->pos.y_pos = fx->pos.y_pos;
            splash->pos.z_pos = fx->pos.z_pos;
            splash->pos.y_rot = 0;
            splash->speed = 0;
            splash->frame_number = 0;
            splash->object_number = SPLASH1;
        }
    }

    ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos < ceiling)
    {
        fx->fallspeed = -fx->fallspeed;
        fx->pos.y_pos = ceiling;
    }

    height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos >= height)
    {
        if (fx->counter)
        {
            fx->speed = 0;
            fx->frame_number = 0;
            fx->counter = 0;
            fx->object_number = EXPLOSION1;
            SoundEffect(105, &fx->pos, 0);
        }
        else
            KillEffect(fx_number);
        return;
    }

    if (ItemNearLara(&fx->pos, fx->counter * 2))
    {
        lara_item->hit_points -= fx->counter;
        lara_item->hit_status = 1;
        if (fx->counter)
        {
            fx->speed = 0;
            fx->frame_number = 0;
            fx->counter = 0;
            fx->object_number = EXPLOSION1;
            SoundEffect(105, &fx->pos, 0);
            lara.spaz_effect_count = 5;
            lara.spaz_effect = fx;
        }
        else
            KillEffect(fx_number);
    }

    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
}
