#include "game.h"

#define MAX_FLARE_AGE (60 * 30)
#define FLARE_YOUNG 30
#define FLARE_OLD (MAX_FLARE_AGE - 60)

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

int DoFlareLight(PHD_VECTOR *pos, int flare_age)
{
    int x, y, z, random;

    if (flare_age >= MAX_FLARE_AGE)
        return 0;

    random = GetRandomDraw();
    x = pos->x + (random & 0xf);
    y = pos->y;
    z = pos->z;

    if (flare_age < FLARE_YOUNG)
        AddDynamicLight(x, y, z, 12 + ((flare_age - FLARE_YOUNG) * 12) / 60, 11);
    else if (flare_age < FLARE_OLD)
        AddDynamicLight(x, y, z, 12, 11);
    else if (random > 0x2000)
        AddDynamicLight(x, y, z, 12 - (random & 3), 11);
    else
    {
        AddDynamicLight(x, y, z, 12, 5);
        return 0;
    }
    return 1;
}

void GetLaraJointAbsPosition(PHD_VECTOR *vec, int joint);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number);

#define UNDERWATER 1

void DoFlareInHand(int flare_age)
{
    PHD_VECTOR pos;

    pos.x = 11;
    pos.y = 32;
    pos.z = 41;
    GetLaraJointAbsPosition(&pos, 13);

    lara.left_arm.flash_gun = DoFlareLight(&pos, flare_age);

    if (lara.flare_age < MAX_FLARE_AGE)
    {
        lara.flare_age++;

        if (room[lara_item->room_number].flags & UNDERWATER)
        {
            SoundEffect(12, &lara_item->pos, UNDERWATER);
            if (GetRandomDraw() < 0x4000)
                CreateBubble((PHD_3DPOS *)&pos, lara_item->room_number);
        }
        else
            SoundEffect(12, &lara_item->pos, 0);
    }
    else if (lara.gun_status == LG_ARMLESS)
        lara.gun_status = LG_UNDRAW;
}

#define FLARE_ITEM 152
#define FLARE_FIRE 235

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void S_CalculateStaticLight(sint16 adder);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);

void DrawFlareInAir(ITEM_INFO *item)
{
    int clip, rate;
    sint16 *frmptr[2];

    GetFrames(item, frmptr, &rate);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0])))
    {
        CalculateObjectLighting(item, frmptr[0]);
        phd_PutPolygons(meshes[objects[FLARE_ITEM].mesh_index], clip);

        if ((sint32)item->data & 0x8000)
        {
            phd_TranslateRel(-6, 6, 80);
            phd_RotX(-90 * ONE_DEGREE);
            phd_RotY((sint16)(GetRandomDraw() * 2));
            S_CalculateStaticLight(8 * 256);
            phd_PutPolygons(meshes[objects[FLARE_FIRE].mesh_index], clip);
        }
    }
    phd_PopMatrix();
}

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);

void CreateFlare(int thrown)
{
    sint16 flare_item;
    ITEM_INFO *flare;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;

    flare_item = CreateItem();
    if (flare_item != NO_ITEM)
    {
        flare = &items[flare_item];

        flare->object_number = FLARE_ITEM;
        flare->room_number = lara_item->room_number;
        pos.x = -16;
        pos.y = 32;
        pos.z = 42;
        GetLaraJointAbsPosition(&pos, 13);

        floor = GetFloor(pos.x, pos.y, pos.z, &flare->room_number);
        if (GetHeight(floor, pos.x, pos.y, pos.z) < pos.y)
        {
            flare->pos.x_pos = lara_item->pos.x_pos;
            flare->pos.y_pos = pos.y;
            flare->pos.z_pos = lara_item->pos.z_pos;
            flare->pos.y_rot = -lara_item->pos.y_rot;
            flare->room_number = lara_item->room_number;
        }
        else
        {
            flare->pos.x_pos = pos.x;
            flare->pos.y_pos = pos.y;
            flare->pos.z_pos = pos.z;
            if (thrown)
                flare->pos.y_rot = lara_item->pos.y_rot;
            else
                flare->pos.y_rot = lara_item->pos.y_rot - 0x2000;
        }

        InitialiseItem(flare_item);

        flare->pos.z_rot = 0;
        flare->pos.x_rot = 0;
        flare->shade = -1;

        if (thrown)
        {
            flare->speed = lara_item->speed + 50;
            flare->fallspeed = lara_item->fallspeed - 50;
        }
        else
        {
            flare->speed = lara_item->speed + 10;
            flare->fallspeed = lara_item->fallspeed + 50;
        }

        if (DoFlareLight((PHD_VECTOR *)&flare->pos, lara.flare_age))
            flare->data = (void *)(lara.flare_age | 0x8000);
        else
            flare->data = (void *)(lara.flare_age & 0x7fff);

        AddActiveItem(flare_item);
        flare->status = ACTIVE;
    }
}

void set_flare_arm(int frame)
{
    int anim_base;

    anim_base = objects[FLARE].anim_index;
    if (frame < FL_THROW_F)
        lara.left_arm.anim_number = anim_base;
    else if (frame < FL_DRAW_F)
        lara.left_arm.anim_number = anim_base + 1;
    else if (frame < FL_IGNITE_F)
        lara.left_arm.anim_number = anim_base + 2;
    else if (frame < FL_2HOLD_F)
        lara.left_arm.anim_number = anim_base + 3;
    else
        lara.left_arm.anim_number = anim_base + 4;
    lara.left_arm.frame_base = anims[lara.left_arm.anim_number].frame_ptr;
}

void draw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[FLARE].mesh_index + HAND_L];
}

void undraw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void ready_flare(void)
{
    lara.gun_status = LG_ARMLESS;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target = NULL;
}
