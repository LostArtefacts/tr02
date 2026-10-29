#include "game.h"

#define HAIR_SEGMENTS 6

static int first_hair;
static PHD_3DPOS hair[HAIR_SEGMENTS + 1];
static PHD_VECTOR hvel[HAIR_SEGMENTS + 1];

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
