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
#define BODY_PART 242
#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);
sint16 *GetBestFrame(ITEM_INFO *item);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotZ(PHD_ANGLE rz);
extern sint32 LsAdder;

void __cdecl phd_PushUnitMatrix(void);

int phd_TranslateRel(sint32 x, sint32 y, sint32 z);

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage)
{
    int i, poppush;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    sint32 *bone, bit;
    sint16 *rotation;
    sint16 *frame, *extra_rotation;
    sint16 fx_number;
    FX_INFO *fx;

    item = &items[item_number];
    object = &objects[item->object_number];
    *(phd_mxptr + M23) = 0;
    S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    frame = GetBestFrame(item);
    phd_PushUnitMatrix();
    *(phd_mxptr + M03) = 0;
    *(phd_mxptr + M13) = 0;
    *(phd_mxptr + M23) = 0;
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    phd_TranslateRel(*(frame + 6), *(frame + 7), *(frame + 8));
    rotation = frame + 9;
    phd_RotYXZsuperpack(&rotation, 0);
    bone = bones + object->bone_index;
    extra_rotation = (sint16 *)item->data;
    bit = 1;
    if ((bit & mesh_bits) && (bit & item->mesh_bits))
    {
        fx_number = CreateEffect(item->room_number);
        if (fx_number != NO_ITEM)
        {
            fx = &effects[fx_number];
            fx->pos.x_pos = item->pos.x_pos + (*(phd_mxptr + M03) >> W2V_SHIFT);
            fx->pos.y_pos = item->pos.y_pos + (*(phd_mxptr + M13) >> W2V_SHIFT);
            fx->pos.z_pos = item->pos.z_pos + (*(phd_mxptr + M23) >> W2V_SHIFT);
            fx->room_number = item->room_number;
            fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
            fx->speed = (sint16)GetRandomControl() >> 8;
            fx->fallspeed = (sint16)-GetRandomControl() >> 8;
            fx->counter = damage;
            fx->frame_number = object->mesh_index;
            fx->object_number = BODY_PART;
            fx->shade = LsAdder - 0x300;
        }
        item->mesh_bits -= bit;
    }
    for (i = 1; i < object->nmeshes; i++, bone += 3)
    {
        poppush = *(bone++);
        if (poppush & 1)
            phd_PopMatrix();
        if (poppush & 2)
            phd_PushMatrix();
        phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
        phd_RotYXZsuperpack(&rotation, 0);
        if (poppush & (ROT_X | ROT_Y | ROT_Z))
        {
            if (poppush & ROT_Y)
                phd_RotY(*(extra_rotation++));
            if (poppush & ROT_X)
                phd_RotX(*(extra_rotation++));
            if (poppush & ROT_Z)
                phd_RotZ(*(extra_rotation++));
        }
        bit <<= 1;
        if ((bit & mesh_bits) && (bit & item->mesh_bits))
        {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM)
            {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos + (*(phd_mxptr + M03) >> W2V_SHIFT);
                fx->pos.y_pos = item->pos.y_pos + (*(phd_mxptr + M13) >> W2V_SHIFT);
                fx->pos.z_pos = item->pos.z_pos + (*(phd_mxptr + M23) >> W2V_SHIFT);
                fx->room_number = item->room_number;
                fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
                fx->speed = (sint16)GetRandomControl() >> 8;
                fx->fallspeed = (sint16)-GetRandomControl() >> 8;
                fx->counter = damage;
                fx->frame_number = object->mesh_index + i;
                fx->object_number = BODY_PART;
                fx->shade = LsAdder - 0x300;
            }
            item->mesh_bits -= bit;
        }
    }
    phd_PopMatrix();
    return !(item->mesh_bits & (0x7fffffff >> (31 - object->nmeshes)));
}


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
