#include "game.h"
#include "savegame.h"

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

int phd_TranslateRel(sint32 x, sint32 y, sint32 z);

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

#define FL_DRAWGOTIT_F (13 + FL_DRAW_F)
#define FLAREBOX_ITEM 151

int Inv_RemoveItem(int itemNum);

void draw_flare(void)
{
    sint16 ani;

    if (lara_item->current_anim_state == AS_FLAREPICKUP || lara_item->current_anim_state == AS_PICKUP)
    {
        DoFlareInHand(lara.flare_age);
        lara.flare_control_left = 0;
        lara.left_arm.frame_number = FL_2HOLD_F - 2;
        set_flare_arm(lara.left_arm.frame_number);
        return;
    }

    ani = lara.left_arm.frame_number;
    ani++;

    lara.flare_control_left = 1;

    if (ani < FL_DRAW_F || ani > FL_2HOLD_F - 1)
        ani = FL_DRAW_F;
    else if (ani == FL_DRAWGOTIT_F)
    {
        draw_flare_meshes();
        if (!savegame.bonus_flag)
            Inv_RemoveItem(FLAREBOX_ITEM);
    }
    else if (ani >= FL_IGNITE_F && ani <= FL_2HOLD_F - 2)
    {
        if (ani == FL_IGNITE_F)
        {
            if (room[lara_item->room_number].flags & UNDERWATER)
                SoundEffect(11, &lara_item->pos, UNDERWATER);
            else
                SoundEffect(11, &lara_item->pos, 0);
            lara.flare_age = 0;
        }
        DoFlareInHand(lara.flare_age);
    }
    else if (ani == FL_2HOLD_F - 1)
    {
        ready_flare();
        ani = FL_HOLD_F;
        DoFlareInHand(lara.flare_age);
    }

    lara.left_arm.frame_number = ani;
    set_flare_arm(lara.left_arm.frame_number);
}

#define FL_THROWRELEASE_F (20 + FL_THROW_F)
#define FL_THROW_FRAMES 32
#define THROWFLARE_A 189
#define THROWFLARE_F GF(THROWFLARE_A, 0)

void InitialiseNewWeapon(void);

void undraw_flare(void)
{
    sint16 ani, ani2;

    ani = lara.left_arm.frame_number;
    ani2 = lara.flare_frame;

    lara.flare_control_left = 1;

    if (lara_item->goal_anim_state == AS_STOP && lara.skidoo == NO_ITEM)
    {
        if (lara_item->anim_number == BREATH_A)
        {
            lara_item->anim_number = THROWFLARE_A;
            lara_item->frame_number = lara.flare_frame = ani2 = THROWFLARE_F + ani;
        }

        if (lara_item->anim_number == THROWFLARE_A)
        {
            lara.flare_control_left = 0;
            if (ani2 >= THROWFLARE_F + FL_THROW_FRAMES - 1)
            {
                lara.gun_type = lara.request_gun_type = lara.last_gun_type;
                lara.gun_status = LG_ARMLESS;
                InitialiseNewWeapon();
                lara.target = NULL;
                lara.left_arm.lock = lara.right_arm.lock = 0;
                lara_item->anim_number = STOP_A;
                lara_item->frame_number = lara.flare_frame = STOP_F;
                lara_item->current_anim_state = AS_STOP;
                lara_item->goal_anim_state = AS_STOP;
                return;
            }

            ani2++;
            lara.flare_frame = ani2;
        }
    }
    else if (lara_item->current_anim_state == AS_STOP && lara.skidoo == NO_ITEM)
    {
        lara_item->anim_number = STOP_A;
        lara_item->frame_number = STOP_F;
    }

    if (ani == FL_HOLD_F)
    {
        ani = FL_THROW_F;
    }
    else if (ani >= FL_IGNITE_F && ani < FL_IGNITE_F + 23)
    {
        ani++;
        if (ani == FL_IGNITE_F + FL_IGNITE_FT - 1)
            ani = FL_THROW_F;
    }
    else if (ani >= FL_THROW_F && ani < FL_DRAW_F)
    {
        ani++;
        if (ani == FL_THROWRELEASE_F)
        {
            CreateFlare(1);
            undraw_flare_meshes();
        }
        else if (ani == FL_DRAW_F)
        {
            ani = 0;
            lara.gun_type = lara.request_gun_type = lara.last_gun_type;
            lara.gun_status = LG_ARMLESS;
            InitialiseNewWeapon();
            lara.target = NULL;
            lara.left_arm.lock = lara.right_arm.lock = 0;
            lara.flare_control_left = 0;
            lara.flare_frame = 0;
        }
    }
    else if (ani >= FL_2HOLD_F && ani < FL_END_F)
    {
        ani++;
        if (ani == FL_END_F)
            ani = FL_THROW_F;
    }

    if (ani >= FL_THROW_F && ani < FL_THROWRELEASE_F)
        DoFlareInHand(lara.flare_age);

    lara.left_arm.frame_number = ani;
    set_flare_arm(lara.left_arm.frame_number);
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

#define GRAVITY 6

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

void FlareControl(sint16 item_number)
{
    ITEM_INFO *flare;
    sint16 room_number;
    sint32 ceiling, height;
    sint32 x, y, z, flare_age;
    FLOOR_INFO *floor;

    flare = &items[item_number];

    if (flare->fallspeed)
    {
        flare->pos.x_rot += ONE_DEGREE * 3;
        flare->pos.z_rot += ONE_DEGREE * 5;
    }
    else
    {
        flare->pos.x_rot = 0;
        flare->pos.z_rot = 0;
    }

    z = flare->pos.z_pos;
    y = flare->pos.y_pos;
    x = flare->pos.x_pos;

    flare->pos.z_pos += flare->speed * phd_cos(flare->pos.y_rot) >> W2V_SHIFT;
    flare->pos.x_pos += flare->speed * phd_sin(flare->pos.y_rot) >> W2V_SHIFT;

    if (room[flare->room_number].flags & UNDERWATER)
    {
        flare->fallspeed += (5 - flare->fallspeed) / 2;
        flare->speed += (5 - flare->speed) / 2;
    }
    else
        flare->fallspeed += GRAVITY;
    flare->pos.y_pos += flare->fallspeed;

    room_number = flare->room_number;
    floor = GetFloor(flare->pos.x_pos, flare->pos.y_pos, flare->pos.z_pos, &room_number);
    height = GetHeight(floor, flare->pos.x_pos, flare->pos.y_pos, flare->pos.z_pos);
    if (flare->pos.y_pos >= height)
    {
        if (y > height)
        {
            flare->pos.y_rot += -32768;
            flare->pos.y_pos = y;
            flare->speed /= 2;
            flare->pos.x_pos = x;
            flare->pos.z_pos = z;
            room_number = flare->room_number;
        }
        else
        {
            if (flare->fallspeed > 40)
            {
                flare->fallspeed = 40 - flare->fallspeed;
                if (flare->fallspeed < -100)
                    flare->fallspeed = -100;
            }
            else
            {
                flare->fallspeed = 0;
                flare->speed -= 3;
                if (flare->speed < 0)
                    flare->speed = 0;
            }
            flare->pos.y_pos = height;
        }
    }
    else
    {
        ceiling = GetCeiling(floor, flare->pos.x_pos, flare->pos.y_pos, flare->pos.z_pos);
        if (flare->pos.y_pos < ceiling)
        {
            flare->fallspeed = -flare->fallspeed;
            flare->pos.y_pos = ceiling;
        }
    }

    if (room_number != flare->room_number)
        ItemNewRoom(item_number, room_number);

    flare_age = (sint32)flare->data & 0x7fff;
    if (flare_age < MAX_FLARE_AGE)
        flare_age++;
    else if (flare->fallspeed == 0 && flare->speed == 0)
    {
        KillItem(item_number);
        return;
    }

    if (DoFlareLight((PHD_VECTOR *)&flare->pos, flare_age))
    {
        flare_age |= 0x8000;
        if (room[flare->room_number].flags & UNDERWATER)
        {
            SoundEffect(12, &flare->pos, UNDERWATER);
            if (GetRandomDraw() < 0x4000)
                CreateBubble(&flare->pos, flare->room_number);
        }
        else
            SoundEffect(12, &flare->pos, 0);
    }

    flare->data = (void *)flare_age;
}
