#include "game.h"
#include "box.h"
#include "savegame.h"

#define BLOOD_RADIUS 256
#define MAX_BOUNCE 100
#define FLIPFLAG 64
#define UNFLIPFLAG 128
#define MESHSWAP1 97
#define MESHSWAP2 98
#define BIRD_TWEETER 215
#define BLOOD1 233
#define BUBBLES1 231
#define RICOCHET1 238
#define SPLASH1 230
#define UNDERWATER 1
#define GRAVITY 6
#define HOT_LIQUID 220
#define WATERFALL_RANGE (WALL_L * 10)

void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);

extern sint32 LsAdder;
void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

#define DRAGON_FIRE 246
#define DRAGON_FRONT 22
#define DRAGON_FLAME_SPEED 200

void ShootAtLara(FX_INFO *fx);

sint16 DragonFire(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
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
        fx->speed = DRAGON_FLAME_SPEED;
        fx->frame_number = GetRandomDraw() * (objects[DRAGON_FIRE].nmeshes + 1) >> 15;
        fx->object_number = DRAGON_FIRE;
        fx->shade = 14 * 256;
        ShootAtLara(fx);

        if (objects[DRAGON_FRONT].loaded)
            fx->counter = 0x4000;
        else
            fx->counter = 20;
    }

    return (fx_number);
}

int ItemNearLara(PHD_3DPOS *pos, sint32 distance)
{
    sint32 x, y, z;
    sint16 *bounds;

    x = pos->x_pos - lara_item->pos.x_pos;
    y = pos->y_pos - lara_item->pos.y_pos;
    z = pos->z_pos - lara_item->pos.z_pos;

    if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_L * 3 || y > WALL_L * 3)
        return 0;

    if (x * x + z * z > distance * distance)
        return 0;

    bounds = GetBoundsAccurate(lara_item);
    if (y < bounds[2] || y > bounds[3] + 100)
        return 0;

    return 1;
}

void SoundEffects(void)
{
    int i;
    OBJECT_VECTOR *sound;

    sound = sound_effects;
    for (i = number_sound_effects; i > 0; i--, sound++)
    {
        if (!flip_status && (sound->flags & UNFLIPFLAG) == 0)
            continue;
        else if (flip_status && (sound->flags & FLIPFLAG) == 0)
            continue;
        SoundEffect(sound->data, (PHD_3DPOS *)&sound->x, 0);
    }
    if (flipeffect != -1)
        (*effect_routines[flipeffect])(NULL);
    SOUND_EndScene();
}

sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE direction, sint16 room_num)
{
    sint16 fx_num;
    FX_INFO *fx;

    fx_num = CreateEffect(room_num);
    if (fx_num != NO_ITEM)
    {
        fx = &effects[fx_num];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->pos.y_rot = direction;
        fx->speed = speed;
        fx->frame_number = 0;
        fx->object_number = BLOOD1;
        fx->counter = 0;
    }
    return fx_num;
}

void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE direction, sint16 room_num, int number)
{
    for (; number > 0; number--)
    {
        DoBloodSplat(x + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000),
                     y + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000),
                     z + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000),
                     speed, direction, room_num);
    }
}

void ControlBlood1(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->pos.x_pos += (phd_sin(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->pos.z_pos += (phd_cos(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->counter++;
    if (fx->counter == 4)
    {
        fx->counter = 0;
        fx->frame_number--;
        if (fx->frame_number <= objects[fx->object_number].nmeshes)
            KillEffect(fx_num);
    }
}

void ControlExplosion1(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->counter++;
    if (fx->counter == 2)
    {
        fx->counter = 0;
        fx->frame_number--;
        if (fx->frame_number <= objects[fx->object_number].nmeshes)
            KillEffect(fx_num);
        else
            AddDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 13, 11);
    }
    else
        AddDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 12, 10);
}

void Richochet(GAME_VECTOR *pos)
{
    sint16 fx_num;
    FX_INFO *fx;

    fx_num = CreateEffect(pos->room_number);
    if (fx_num != NO_ITEM)
    {
        fx = &effects[fx_num];
        fx->pos.x_pos = pos->x;
        fx->pos.y_pos = pos->y;
        fx->pos.z_pos = pos->z;
        fx->counter = 4;
        fx->frame_number = -3 * GetRandomDraw() / 0x8000;
        fx->object_number = RICOCHET1;
        SoundEffect(10, &fx->pos, 0);
    }
}

void ControlRichochet1(sint16 fx_num)
{
    effects[fx_num].counter--;
    if (!effects[fx_num].counter)
        KillEffect(fx_num);
}

void CreateBubble(PHD_3DPOS *pos, sint16 room_number)
{
    sint16 fx_num;
    FX_INFO *fx;

    fx_num = CreateEffect(room_number);
    if (fx_num != NO_ITEM)
    {
        fx = &effects[fx_num];
        fx->pos.x_pos = pos->x_pos;
        fx->pos.y_pos = pos->y_pos;
        fx->pos.z_pos = pos->z_pos;
        fx->speed = 10 + ((GetRandomDraw() * 6) >> 15);
        fx->frame_number = -((GetRandomDraw() * 3) >> 15);
        fx->object_number = BUBBLES1;
    }
}

void LaraBubbles(ITEM_INFO *item)
{
    int count;
    PHD_VECTOR offset;

    count = GetRandomDraw() * 3 / 0x8000;
    if (!count)
        return;

    SoundEffect(37, &item->pos, UNDERWATER);
    offset.x = 0;
    offset.y = 0;
    offset.z = 50;
    GetJointAbsPosition(item, &offset, 14);
    for (; count > 0; count--)
        CreateBubble((PHD_3DPOS *)&offset, item->room_number);
}

void ControlBubble1(sint16 fx_num)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint32 x, y, z, ceiling;
    sint16 room_number;

    fx = &effects[fx_num];
    fx->pos.y_rot += 1638;
    fx->pos.x_rot += 2366;
    x = fx->pos.x_pos + ((phd_sin(fx->pos.y_rot) * 11) >> W2V_SHIFT);
    y = fx->pos.y_pos - fx->speed;
    z = fx->pos.z_pos + ((phd_cos(fx->pos.x_rot) * 8) >> W2V_SHIFT);

    room_number = fx->room_number;
    floor = GetFloor(x, y, z, &room_number);
    if (!floor || !(room[room_number].flags & UNDERWATER))
    {
        KillEffect(fx_num);
        return;
    }

    ceiling = GetCeiling(floor, x, y, z);
    if (ceiling == NO_HEIGHT || y <= ceiling)
    {
        KillEffect(fx_num);
        return;
    }

    if (fx->room_number != room_number)
        EffectNewRoom(fx_num, room_number);
    fx->pos.x_pos = x;
    fx->pos.y_pos = y;
    fx->pos.z_pos = z;
}

void Splash(ITEM_INFO *item)
{
    sint16 fx_num, room_number;
    FX_INFO *fx;
    sint32 wh;
    int i;

    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

    for (i = 0; i < 10; i++)
    {
        fx_num = CreateEffect(room_number);
        if (fx_num != NO_ITEM)
        {
            fx = &effects[fx_num];
            fx->pos.x_pos = item->pos.x_pos;
            fx->pos.y_pos = wh;
            fx->pos.z_pos = item->pos.z_pos;
            fx->pos.y_rot = -32768 + 2 * GetRandomDraw();
            fx->speed = GetRandomDraw() / 256;
            fx->frame_number = 0;
            fx->object_number = SPLASH1;
        }
    }
}

void ControlSplash1(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->frame_number--;
    if (fx->frame_number <= objects[fx->object_number].nmeshes)
    {
        KillEffect(fx_num);
        return;
    }

    fx->pos.z_pos += (phd_cos(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->pos.x_pos += (phd_sin(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
}

void ControlWaterSprite(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->counter--;
    if (!(fx->counter & 3))
    {
        fx->frame_number--;
        if (fx->frame_number <= objects[fx->object_number].nmeshes)
            fx->frame_number = 0;
    }

    if (!fx->counter || fx->fallspeed > 0)
        KillEffect(fx_num);
    else
    {
        fx->pos.z_pos += (phd_cos(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
        fx->pos.x_pos += (phd_sin(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
        if (fx->fallspeed)
        {
            fx->pos.y_pos += fx->fallspeed;
            fx->fallspeed += GRAVITY;
        }
    }
}

void ControlSnowSprite(sint16 fx_num)
{
    FX_INFO *fx;

    fx = &effects[fx_num];
    fx->frame_number--;
    if (fx->frame_number <= objects[fx->object_number].nmeshes)
    {
        KillEffect(fx_num);
        return;
    }

    fx->pos.z_pos += (phd_cos(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->pos.x_pos += (phd_sin(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    if (fx->fallspeed)
    {
        fx->pos.y_pos += fx->fallspeed;
        fx->fallspeed += GRAVITY;
    }
}

void ControlHotLiquid(sint16 fx_num)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 height;

    fx = &effects[fx_num];
    fx->frame_number--;
    if (fx->frame_number <= objects[HOT_LIQUID].nmeshes)
        fx->frame_number = 0;

    fx->pos.y_pos += fx->fallspeed;
    fx->fallspeed += GRAVITY;

    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos >= height)
    {
        SoundEffect(285, &fx->pos, 0);
        fx->object_number = SPLASH1;
        fx->pos.y_pos = height;
        fx->pos.y_rot = GetRandomDraw() * 2;
        fx->fallspeed = 0;
        fx->speed = 50;
        return;
    }

    if (fx->room_number != room_number)
        EffectNewRoom(fx_num, room_number);
    SoundEffect(284, &fx->pos, 0);
}

void WaterFall(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_num;
    sint32 x, y, z;

    item = &items[item_number];
    x = item->pos.x_pos - lara_item->pos.x_pos;
    y = item->pos.y_pos - lara_item->pos.y_pos;
    z = item->pos.z_pos - lara_item->pos.z_pos;

    if (x >= -WATERFALL_RANGE && x <= WATERFALL_RANGE && z >= -WATERFALL_RANGE && z <= WATERFALL_RANGE
        && y >= -WATERFALL_RANGE && y <= WATERFALL_RANGE)
    {
        S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
        fx_num = CreateEffect(item->room_number);
        if (fx_num != NO_ITEM)
        {
            fx = &effects[fx_num];
            fx->pos.x_pos = item->pos.x_pos + (GetRandomDraw() - 0x4000) * WALL_L / 0x7fff;
            fx->pos.z_pos = item->pos.z_pos + (GetRandomDraw() - 0x4000) * WALL_L / 0x7fff;
            fx->pos.y_pos = item->pos.y_pos;
            fx->speed = 0;
            fx->shade = LsAdder;
            fx->frame_number = 0;
            fx->object_number = SPLASH1;
        }
        SoundEffect(79, &item->pos, 0);
    }
}

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

void lara_normal_effect(ITEM_INFO *item)
{
    item->current_anim_state = AS_STOP;
    item->goal_anim_state = AS_STOP;
    item->frame_number = STOP_F;
    item->anim_number = STOP_A;
    camera.type = CHASE_CAMERA;
    AlterFOV(GAME_FOV * ONE_DEGREE);
}

void BoilerFX(ITEM_INFO *item)
{
    SoundEffect(338, NULL, 0);
    flipeffect = -1;
}

void FloodFX(ITEM_INFO *item)
{
    PHD_3DPOS pos;

    if (fliptimer > 30 * 4)
        flipeffect = -1;
    else
    {
        pos.x_pos = lara_item->pos.x_pos;
        if (fliptimer < 30)
            pos.y_pos = camera.target.y + (30 - fliptimer) * 100;
        else
            pos.y_pos = camera.target.y + (fliptimer - 30) * 100;
        pos.z_pos = lara_item->pos.z_pos;
        SoundEffect(79, &pos, 0);
    }
    fliptimer++;
}

void RubbleFX(ITEM_INFO *item)
{
    SoundEffect(24, NULL, 0);
    camera.bounce = -350;
    flipeffect = -1;
}

void ChandelierFX(ITEM_INFO *item)
{
    SoundEffect(278, NULL, 0);
    fliptimer++;
    if (fliptimer > 30)
        flipeffect = -1;
}

void ExplosionFX(ITEM_INFO *item)
{
    SoundEffect(105, NULL, 0);
    camera.bounce = -75;
    flipeffect = -1;
}

void PistonFX(ITEM_INFO *item)
{
    SoundEffect(190, NULL, 0);
    flipeffect = -1;
}

void CurtainFX(ITEM_INFO *item)
{
    SoundEffect(191, NULL, 0);
    flipeffect = -1;
}

void StatueFX(ITEM_INFO *item)
{
    SoundEffect(331, NULL, 0);
    flipeffect = -1;
}

void SetChangeFX(ITEM_INFO *item)
{
    SoundEffect(330, NULL, 0);
    flipeffect = -1;
}

void ControlDingDong(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if ((item->flags & CODE_BITS) == CODE_BITS)
    {
        SoundEffect(334, &item->pos, 0);
        item->flags -= CODE_BITS;
    }
}

void ControlLaraAlarm(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if ((item->flags & CODE_BITS) == CODE_BITS)
        SoundEffect(335, &item->pos, 0);
}

void ControlAlarmSound(sint16 item_number)
{
    int timer;
    ITEM_INFO *item;

    item = &items[item_number];
    if ((item->flags & CODE_BITS) == CODE_BITS)
    {
        SoundEffect(332, &item->pos, 0);
        timer = (int)item->data;
        timer++;
        if (timer > 6)
        {
            AddDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 12, 11);
            if (timer > 12)
                timer = 0;
        }
        item->data = (void *)timer;
    }
}

void ControlBirdTweeter(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == BIRD_TWEETER)
    {
        if (GetRandomDraw() < 0x400)
            SoundEffect(316, &item->pos, 0);
    }
    else if (GetRandomDraw() < 0x100)
        SoundEffect(329, &item->pos, 0);
}

void DoChimeSound(ITEM_INFO *item)
{
    PHD_3DPOS pos;

    pos.x_pos = lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 6);
    pos.y_pos = lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 6);
    pos.z_pos = lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 6);

    SoundEffect(208, &pos, 0);
}

void ControlClockChimes(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->timer == 0)
        return;

    if ((item->timer % 60) == 59)
        DoChimeSound(item);
    item->timer--;

    if (item->timer == 0)
    {
        DoChimeSound(item);
        item->timer = -1;
        RemoveActiveItem(item_number);
        item->status = NOT_ACTIVE;
        item->flags &= ~CODE_BITS;
    }
}

#define SPHERE_OF_DOOM2 210
#define SPHERE_RADIUS (STEP_L * 5 / 2)
#define FALLDOWN_A 34

void SphereOfDoomCollision(sint16 item_number, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    int radius, x, z, dist;
    ITEM_INFO *item;
    sint16 angle, diff;

    if (room[laraitem->room_number].flags & UNDERWATER)
        return;

    item = &items[item_number];

    radius = item->timer * SPHERE_RADIUS >> 8;
    x = laraitem->pos.x_pos - item->pos.x_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    dist = x * x + z * z;

    if (dist < radius * radius)
    {
        angle = phd_atan(z, x);
        diff = laraitem->pos.y_rot - angle;
        if (ABS(diff) < 0x4000)
        {
            laraitem->speed = 150;
            laraitem->pos.y_rot = angle;
        }
        else
        {
            laraitem->speed = -150;
            laraitem->pos.y_rot = (sint16)(angle - 0x8000);
        }
        laraitem->gravity_status = 1;
        laraitem->fallspeed = -50;
        laraitem->pos.z_pos = item->pos.z_pos + ((radius + 50) * phd_cos(angle) >> W2V_SHIFT);
        laraitem->pos.x_pos = item->pos.x_pos + ((radius + 50) * phd_sin(angle) >> W2V_SHIFT);
        laraitem->pos.x_rot = laraitem->pos.z_rot = 0;
        lara_item->anim_number = FALLDOWN_A;
        lara_item->frame_number = anims[FALLDOWN_A].frame_base;
        lara_item->current_anim_state = AS_FORWARDJUMP;
        lara_item->goal_anim_state = AS_FORWARDJUMP;
    }
}

void SphereOfDoom(sint16 item_number)
{
    ITEM_INFO *item;
    PHD_3DPOS pos;
    int dist, x, y, z, radius;

    item = &items[item_number];

    item->timer += 64;
    item->pos.y_rot += (item->object_number == SPHERE_OF_DOOM2) ? ONE_DEGREE * 10 : -ONE_DEGREE * 10;

    x = item->pos.x_pos - lara_item->pos.x_pos;
    y = item->pos.y_pos - lara_item->pos.y_pos;
    z = item->pos.z_pos - lara_item->pos.z_pos;

    radius = item->timer * SPHERE_RADIUS >> 8;
    dist = phd_sqrt(x * x + y * y + z * z) - radius;

    pos.x_pos = lara_item->pos.x_pos + x * dist / radius;
    pos.y_pos = lara_item->pos.y_pos + y * dist / radius;
    pos.z_pos = lara_item->pos.z_pos + z * dist / radius;
    SoundEffect(341, &pos, 0);

    if (item->timer > (30 << 7))
        KillItem(item_number);
}

int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);

void DrawSphereOfDoom(ITEM_INFO *item)
{
    int scale, clip;
    sint16 *frame;

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotY(item->pos.y_rot);

    scale = item->timer;
    *(phd_mxptr + M00) = *(phd_mxptr + M00) * scale >> 8;
    *(phd_mxptr + M01) = *(phd_mxptr + M01) * scale >> 8;
    *(phd_mxptr + M02) = *(phd_mxptr + M02) * scale >> 8;
    *(phd_mxptr + M10) = *(phd_mxptr + M10) * scale >> 8;
    *(phd_mxptr + M11) = *(phd_mxptr + M11) * scale >> 8;
    *(phd_mxptr + M12) = *(phd_mxptr + M12) * scale >> 8;
    *(phd_mxptr + M20) = *(phd_mxptr + M20) * scale >> 8;
    *(phd_mxptr + M21) = *(phd_mxptr + M21) * scale >> 8;
    *(phd_mxptr + M22) = *(phd_mxptr + M22) * scale >> 8;

    frame = anims[item->anim_number].frame_ptr;
    clip = S_GetObjectBounds(frame);
    if (clip)
    {
        CalculateObjectLighting(item, frame);
        phd_PutPolygons(meshes[objects[item->object_number].mesh_index], clip);
    }

    phd_PopMatrix();
}

void lara_hands_free(ITEM_INFO *item)
{
    lara.gun_status = LG_ARMLESS;
}

void flip_map_effect(ITEM_INFO *item)
{
    FlipMap();
}

void draw_right_gun(ITEM_INFO *item)
{
    sint16 *temp;

    temp = lara.mesh_ptrs[THIGH_R];
    lara.mesh_ptrs[THIGH_R] = meshes[objects[PISTOLS].mesh_index + THIGH_R];
    meshes[objects[PISTOLS].mesh_index + THIGH_R] = temp;
    temp = lara.mesh_ptrs[HAND_R];
    lara.mesh_ptrs[HAND_R] = meshes[objects[PISTOLS].mesh_index + HAND_R];
    meshes[objects[PISTOLS].mesh_index + HAND_R] = temp;
}

void draw_left_gun(ITEM_INFO *item)
{
    sint16 *temp;

    temp = lara.mesh_ptrs[THIGH_L];
    lara.mesh_ptrs[THIGH_L] = meshes[objects[PISTOLS].mesh_index + THIGH_L];
    meshes[objects[PISTOLS].mesh_index + THIGH_L] = temp;
    temp = lara.mesh_ptrs[HAND_L];
    lara.mesh_ptrs[HAND_L] = meshes[objects[PISTOLS].mesh_index + HAND_L];
    meshes[objects[PISTOLS].mesh_index + HAND_L] = temp;
}

void swap_meshes_with_meshswap1(ITEM_INFO *item)
{
    int i;
    sint16 nmeshes;
    sint16 *temp;
    OBJECT_INFO *obj;

    obj = &objects[item->object_number];
    nmeshes = obj->nmeshes;
    for (i = 0; i < obj->nmeshes; i++)
    {
        temp = meshes[obj->mesh_index + i];
        meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP1].mesh_index + i];
        meshes[objects[MESHSWAP1].mesh_index + i] = temp;
    }
}

void swap_meshes_with_meshswap2(ITEM_INFO *item)
{
    int i;
    sint16 nmeshes;
    sint16 *temp;
    OBJECT_INFO *obj;

    obj = &objects[item->object_number];
    nmeshes = obj->nmeshes;
    for (i = 0; i < obj->nmeshes; i++)
    {
        temp = meshes[obj->mesh_index + i];
        meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP2].mesh_index + i];
        meshes[objects[MESHSWAP2].mesh_index + i] = temp;
    }
}

void invisibility_on(ITEM_INFO *item)
{
    item->status = INVISIBLE;
}

void invisibility_off(ITEM_INFO *item)
{
    item->status = ACTIVE;
}

void dynamic_light_on(ITEM_INFO *item)
{
    item->dynamic_light = 1;
}

void dynamic_light_off(ITEM_INFO *item)
{
    item->dynamic_light = 0;
}

void reset_hair(ITEM_INFO *item)
{
    InitialiseHair();
}

void AssaultStart(ITEM_INFO *item)
{
    savegame.timer = 0;
    assault_timer_active = 1;
    assault_timer_display = 1;
    flipeffect = -1;
}

void AssaultStop(ITEM_INFO *item)
{
    assault_timer_active = 0;
    assault_timer_display = 1;
    flipeffect = -1;
}

void AssaultReset(ITEM_INFO *item)
{
    assault_timer_active = 0;
    assault_timer_display = 0;
    flipeffect = -1;
}

void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite)
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    DoBloodSplat(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}
