#include "game.h"
#include "larafire.h"

#define FLARE_FIRE 235
#define GUN_FLASH 240
#define M16_FLASH 241

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void S_CalculateStaticLight(sint16 adder);

#define S_LongMemCpy(d, s, n) memcpy(d, s, (n) * sizeof(sint32))

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

void InitInterpolate(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;

    IMptr = IMstack;
    S_LongMemCpy(IMstack, phd_mxptr, 12);
}

void phd_PopMatrix_I(void)
{
    phd_PopMatrix();
    IMptr -= 12;
}

void phd_PushMatrix_I(void)
{
    phd_PushMatrix();
    S_LongMemCpy(IMptr + 12, IMptr, 12);
    IMptr += 12;
}

void phd_RotY_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotY(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotY(ang);
    phd_mxptr = oldm;
}

void phd_RotX_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotX(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotX(ang);
    phd_mxptr = oldm;
}

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

extern sint16 interpolated_bounds[6];

sint16 *GetBoundsAccurate(ITEM_INFO *item)
{
    int i, frac, rate;
    sint16 *bptr, *frmptr[2];

    if ((frac = GetFrames(item, frmptr, &rate)) == 0)
        return frmptr[0];
    else
    {
        bptr = interpolated_bounds;

        for (i = 0; i < 6; i++, bptr++, frmptr[0]++, frmptr[1]++)
            *(bptr) = *(frmptr[0]) + ((*(frmptr[1]) - *(frmptr[0])) * frac) / rate;

        return interpolated_bounds;
    }
}

sint16 *GetBestFrame(ITEM_INFO *item)
{
    int frac, rate;
    sint16 *frmptr[2];

    frac = GetFrames(item, frmptr, &rate);

    if (frac <= rate / 2)
        return frmptr[0];
    else
        return frmptr[1];
}

#define MAX_DYNAMICS 10

typedef struct {
    sint32 x, y, z;
    sint16 intensity, intensity2;
    sint32 falloff, falloff2;
} LIGHT_INFO;

extern sint32 number_dynamics;
extern LIGHT_INFO dynamic[MAX_DYNAMICS];

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff)
{
    LIGHT_INFO *dl;

    if (number_dynamics >= MAX_DYNAMICS)
        dl = dynamic;
    else
        dl = &dynamic[number_dynamics++];

    dl->x = x;
    dl->y = y;
    dl->z = z;
    dl->intensity = (sint16)intensity;
    dl->falloff = falloff;
}
