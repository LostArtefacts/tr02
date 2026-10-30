#include "game.h"

#define BLOOD_RADIUS 256
#define MAX_BOUNCE 100
#define FLIPFLAG 64
#define UNFLIPFLAG 128

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
