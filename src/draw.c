#include "game.h"
#include "larafire.h"

#define FLARE_FIRE 235
#define GUN_FLASH 240
#define M16_FLASH 241

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void S_CalculateStaticLight(sint16 adder);

void DrawGunFlash(int weapon_type, int clip)
{
    sint16 light;
    int g_len, nZOff;

    switch (weapon_type)
    {
    case LG_FLARE:
        phd_TranslateRel(11, 32, 80);
        phd_RotX(-90 * ONE_DEGREE);
        phd_RotY((sint16)(GetRandomDraw() * 2));
        S_CalculateStaticLight(2048);
        phd_PutPolygons(meshes[objects[FLARE_FIRE].mesh_index], clip);
        return;

    case LG_MAGNUMS:
        light = 4096;
        g_len = 215;
        nZOff = 65;
        break;

    case LG_UZIS:
        light = 2560;
        g_len = 200;
        nZOff = 50;
        break;

    case LG_SHOTGUN:
        return;

    case LG_M16:
        phd_TranslateRel(0, 400, 99);
        phd_RotYXZ(0, -85 * ONE_DEGREE, (sint16)((GetRandomDraw() * 2 & 0x4000) + 0x2000));
        S_CalculateStaticLight(2560);
        phd_PutPolygons(meshes[objects[M16_FLASH].mesh_index], clip);
        return;

    default:
        light = 5120;
        g_len = 185;
        nZOff = 40;
        break;
    }

    phd_TranslateRel(0, g_len, nZOff);
    phd_RotYXZ(0, -90 * ONE_DEGREE, (sint16)(GetRandomDraw() * 2));
    S_CalculateStaticLight(light);
    phd_PutPolygons(meshes[objects[GUN_FLASH].mesh_index], clip);
}

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate)
{
    ANIM_STRUCT *anim;
    int frm;
    int first, second;
    int frame_size;
    int interp, rat;

    frm = item->frame_number;
    anim = &anims[item->anim_number];
    frmptr[0] = frmptr[1] = anim->frame_ptr;
    rat = *rate = anim->interpolation & 0x00ff;
    frame_size = anim->interpolation >> 8;
    frm -= anim->frame_base;
    first = frm / rat;
    interp = frm % rat;
    frmptr[0] += first * frame_size;
    frmptr[1] = frmptr[0] + frame_size;
    if (interp == 0)
        return 0;
    second = first * rat + rat;
    if (second > anim->frame_end)
        *rate = anim->frame_end - (second - rat);
    return interp;
}
