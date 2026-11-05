#include <windows.h>
#include "game.h"

#define HAIR_SEGMENTS 6

extern int first_hair;
extern PHD_3DPOS hair[HAIR_SEGMENTS + 1];
extern PHD_VECTOR hvel[HAIR_SEGMENTS + 1];

void InitialiseHair(void)
{
    int i;
    sint32 *bone;

    first_hair = 1;
    bone = bones + objects[HAIR].bone_index;
    hair[0].y_rot = 0;
    hair[0].x_rot = -0x4000;
    for (i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
    {
        hair[i].x_pos = *(bone + 1);
        hair[i].y_pos = *(bone + 2);
        hair[i].z_pos = *(bone + 3);
        hair[i].x_rot = -0x4000;
        hair[i].y_rot = hair[i].z_rot = 0;
        hvel[i].x = hvel[i].y = hvel[i].z = 0;
    }
}

#define NOT_INSIDE 32
#define SQUARE(x) ((x) * (x))

void __cdecl phd_PushUnitMatrix(void);
sint16 *GetBestFrame(ITEM_INFO *item);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);

void HairControl(int in_cutscene)
{
    OBJECT_INFO *object;
    sint32 *bone, distance;
    sint16 *rotation;
    sint16 *frame, *objptr, room_number;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;
    sint32 i, water_level, height, size;
    SPHERE sphere[5];
    sint32 j, x, y, z;
    static int wind = 0;

    object = &objects[LARA];
    if (lara.hit_direction >= 0)
    {
        switch (lara.hit_direction)
        {
        default:
        case 0:
            frame = anims[125].frame_ptr;
            size = anims[125].interpolation >> 8;
            break;
        case 1:
            frame = anims[127].frame_ptr;
            size = anims[127].interpolation >> 8;
            break;
        case 2:
            frame = anims[126].frame_ptr;
            size = anims[126].interpolation >> 8;
            break;
        case 3:
            frame = anims[128].frame_ptr;
            size = anims[128].interpolation >> 8;
            break;
        }
        frame += (int)(lara.hit_frame * size);
    }
    else
        frame = GetBestFrame(lara_item);

    phd_PushUnitMatrix();
    *(phd_mxptr + M03) = lara_item->pos.x_pos << W2V_SHIFT;
    *(phd_mxptr + M13) = lara_item->pos.y_pos << W2V_SHIFT;
    *(phd_mxptr + M23) = lara_item->pos.z_pos << W2V_SHIFT;
    phd_RotYXZ(lara_item->pos.y_rot, lara_item->pos.x_rot, lara_item->pos.z_rot);

    rotation = frame + 9;
    bone = bones + object->bone_index;
    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    phd_RotYXZsuperpack(&rotation, 0);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[HIPS];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[0].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[0].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[0].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[0].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0
            || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4))
    {
        rotation = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        phd_RotYXZsuperpack(&rotation, 7);
    }
    else
        phd_RotYXZsuperpack(&rotation, 6);
    phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[TORSO];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[1].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[1].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[1].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[1].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
    phd_RotYXZsuperpack(&rotation, 0);
    objptr = lara.mesh_ptrs[UARM_R];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[3].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[3].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[3].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[3].r = (sint32)*(objptr + 3) * 3 / 2;
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
    phd_RotYXZsuperpack(&rotation, 2);
    objptr = lara.mesh_ptrs[UARM_R];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[4].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[4].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[4].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[4].r = (sint32)*(objptr + 3) * 3 / 2;
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    phd_RotYXZsuperpack(&rotation, 2);
    phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[HEAD];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[2].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[2].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[2].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[2].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_TranslateRel(0, -23, -55);
    pos.x = *(phd_mxptr + M03) >> W2V_SHIFT;
    pos.y = *(phd_mxptr + M13) >> W2V_SHIFT;
    pos.z = *(phd_mxptr + M23) >> W2V_SHIFT;
    phd_PopMatrix();

    bone = bones + objects[HAIR].bone_index;

    if (first_hair)
    {
        first_hair = 0;
        hair[0].x_pos = pos.x;
        hair[0].y_pos = pos.y;
        hair[0].z_pos = pos.z;
        for (i = 0; i < HAIR_SEGMENTS; i++, bone += 4)
        {
            phd_PushUnitMatrix();
            *(phd_mxptr + M03) = hair[i].x_pos << W2V_SHIFT;
            *(phd_mxptr + M13) = hair[i].y_pos << W2V_SHIFT;
            *(phd_mxptr + M23) = hair[i].z_pos << W2V_SHIFT;
            phd_RotYXZ(hair[i].y_rot, hair[i].x_rot, 0);
            phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            hair[i + 1].x_pos = *(phd_mxptr + M03) >> W2V_SHIFT;
            hair[i + 1].y_pos = *(phd_mxptr + M13) >> W2V_SHIFT;
            hair[i + 1].z_pos = *(phd_mxptr + M23) >> W2V_SHIFT;
            phd_PopMatrix();
        }
        wind = 0;
    }
    else
    {
        hair[0].x_pos = pos.x;
        hair[0].y_pos = pos.y;
        hair[0].z_pos = pos.z;

        room_number = lara_item->room_number;
        if (in_cutscene)
            water_level = NO_HEIGHT;
        else
        {
            x = lara_item->pos.x_pos + (frame[0] + frame[1]) / 2;
            y = lara_item->pos.y_pos + (frame[2] + frame[3]) / 2;
            z = lara_item->pos.z_pos + (frame[4] + frame[5]) / 2;
            water_level = GetWaterHeight(x, y, z, room_number);
        }

        floor = GetFloor(hair[0].x_pos, hair[0].y_pos, hair[0].z_pos, &room_number);
        height = GetHeight(floor, hair[0].x_pos, hair[0].y_pos, hair[0].z_pos);
        if (height < hair[0].y_pos)
            height = lara_item->floor;

        if (room[room_number].flags & NOT_INSIDE)
        {
            i = GetRandomDraw() & 7;
            if (i)
            {
                wind += i - 4;
                if (wind < 0)
                    wind = 0;
                else if (wind >= 8)
                    wind--;
            }
        }
        else
            wind = 0;

        for (i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
        {
            hvel[0].x = hair[i].x_pos;
            hvel[0].y = hair[i].y_pos;
            hvel[0].z = hair[i].z_pos;

            hair[i].x_pos += hvel[i].x * 3 / 4;
            hair[i].y_pos += hvel[i].y * 3 / 4;
            hair[i].z_pos += hvel[i].z * 3 / 4;

            switch (lara.water_status)
            {
            case LARA_ABOVEWATER:
                hair[i].y_pos += 10;
                if (water_level != NO_HEIGHT && hair[i].y_pos > water_level)
                    hair[i].y_pos = water_level;
                else if (hair[i].y_pos > height)
                    hair[i].y_pos = height;
                else
                    hair[i].z_pos += wind;
                break;

            case LARA_UNDERWATER:
            case LARA_SURFACE:
            case LARA_WADE:
                if (hair[i].y_pos < water_level)
                    hair[i].y_pos = water_level;
                else if (hair[i].y_pos > height)
                    hair[i].y_pos = height;
                break;
            }

            for (j = 0; j < 5; j++)
            {
                x = hair[i].x_pos - sphere[j].x;
                y = hair[i].y_pos - sphere[j].y;
                z = hair[i].z_pos - sphere[j].z;
                distance = x * x + y * y + z * z;
                if (distance < SQUARE(sphere[j].r))
                {
                    distance = phd_sqrt(distance);
                    if (distance == 0)
                        distance = 1;
                    hair[i].x_pos = sphere[j].x + x * sphere[j].r / distance;
                    hair[i].y_pos = sphere[j].y + y * sphere[j].r / distance;
                    hair[i].z_pos = sphere[j].z + z * sphere[j].r / distance;
                }
            }

            hair[i - 1].y_rot = phd_atan(hair[i].z_pos - hair[i - 1].z_pos, hair[i].x_pos - hair[i - 1].x_pos);
            distance = phd_sqrt(SQUARE(hair[i].z_pos - hair[i - 1].z_pos) + SQUARE(hair[i].x_pos - hair[i - 1].x_pos));
            hair[i - 1].x_rot = -phd_atan(distance, hair[i].y_pos - hair[i - 1].y_pos);

            phd_PushUnitMatrix();
            *(phd_mxptr + M03) = hair[i - 1].x_pos << W2V_SHIFT;
            *(phd_mxptr + M13) = hair[i - 1].y_pos << W2V_SHIFT;
            *(phd_mxptr + M23) = hair[i - 1].z_pos << W2V_SHIFT;
            phd_RotYXZ(hair[i - 1].y_rot, hair[i - 1].x_rot, 0);
            if (i == HAIR_SEGMENTS)
                phd_TranslateRel(*(bone - 3), *(bone - 2), *(bone - 1));
            else
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            hair[i].x_pos = *(phd_mxptr + M03) >> W2V_SHIFT;
            hair[i].y_pos = *(phd_mxptr + M13) >> W2V_SHIFT;
            hair[i].z_pos = *(phd_mxptr + M23) >> W2V_SHIFT;

            hvel[i].x = hair[i].x_pos - hvel[0].x;
            hvel[i].y = hair[i].y_pos - hvel[0].y;
            hvel[i].z = hair[i].z_pos - hvel[0].z;
            phd_PopMatrix();
        }
    }
}

void DrawHair(void)
{
    int i;
    OBJECT_INFO *object;
    sint16 **mesh;

    object = &objects[HAIR];
    mesh = &meshes[object->mesh_index];
    for (i = 0; i < HAIR_SEGMENTS; i++)
    {
        phd_PushMatrix();
        phd_TranslateAbs(hair[i].x_pos, hair[i].y_pos, hair[i].z_pos);
        phd_RotY(hair[i].y_rot);
        phd_RotX(hair[i].x_rot);
        phd_PutPolygons(*mesh++, 1);
        phd_PopMatrix();
    }
}

#ifdef TR2_EXE
int first_hair;
PHD_VECTOR hvel[HAIR_SEGMENTS + 1];
PHD_3DPOS hair[HAIR_SEGMENTS + 1];
#endif
