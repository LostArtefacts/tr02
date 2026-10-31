#include "../specific/global.h"
#include "../3dglodef.h"
#include "../specific/winmain.h"

extern float LfAspectCorrection;

void phd_GenerateW2V(PHD_3DPOS *viewpos)
{
    sint32 sx, sy, sz;
    sint32 cx, cy, cz;
    sint32 *mptr, *nptr;

    mptr = phd_mxptr = matrix_stack;
    nptr = w2v_matrix;
    sx = phd_sin(viewpos->x_rot);
    cx = phd_cos(viewpos->x_rot);
    sy = phd_sin(viewpos->y_rot);
    cy = phd_cos(viewpos->y_rot);
    sz = phd_sin(viewpos->z_rot);
    cz = phd_cos(viewpos->z_rot);
    *(nptr + M00) = *(mptr + M00) = (TRIGMULT3(sx, sy, sz) + TRIGMULT2(cy, cz));
    *(nptr + M01) = *(mptr + M01) = (TRIGMULT2(cx, sz));
    *(nptr + M02) = *(mptr + M02) = (TRIGMULT3(sx, cy, sz) - TRIGMULT2(sy, cz));
    *(nptr + M10) = *(mptr + M10) = (TRIGMULT3(sx, sy, cz) - TRIGMULT2(cy, sz));
    *(nptr + M11) = *(mptr + M11) = (TRIGMULT2(cx, cz));
    *(nptr + M12) = *(mptr + M12) = (TRIGMULT3(sx, cy, cz) + TRIGMULT2(sy, sz));
    *(nptr + M20) = *(mptr + M20) = (TRIGMULT2(cx, sy));
    *(nptr + M21) = *(mptr + M21) = -(sx);
    *(nptr + M22) = *(mptr + M22) = (TRIGMULT2(cx, cy));
    *(nptr + M03) = *(mptr + M03) = viewpos->x_pos;
    *(nptr + M13) = *(mptr + M13) = viewpos->y_pos;
    *(nptr + M23) = *(mptr + M23) = viewpos->z_pos;
    *(nptr + M10) = *(mptr + M10) = (sint32)(float(*(mptr + M10)) * LfAspectCorrection);
    *(nptr + M11) = *(mptr + M11) = (sint32)(float(*(mptr + M11)) * LfAspectCorrection);
    *(nptr + M12) = *(mptr + M12) = (sint32)(float(*(mptr + M12)) * LfAspectCorrection);
}

extern "C" void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll)
{
    PHD_ANGLE angles[2];
    PHD_3DPOS viewer;

    phd_GetVectorAngles(xtar - xsrc, ytar - ysrc, ztar - zsrc, angles);
    viewer.x_pos = xsrc;
    viewer.y_pos = ysrc;
    viewer.z_pos = zsrc;
    viewer.x_rot = angles[1];
    viewer.y_rot = angles[0];
    viewer.z_rot = roll;
    phd_GenerateW2V(&viewer);
}

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest)
{
    PHD_ANGLE pitch;

    *(dest + 0) = phd_atan(z, x);
    while ((sint16)x != x || (sint16)y != y || (sint16)z != z)
    {
        x >>= 2;
        y >>= 2;
        z >>= 2;
    }
    pitch = phd_atan(phd_sqrt(x * x + z * z), y);
    if ((y > 0 && pitch > 0) || (y < 0 && pitch < 0))
        pitch = -pitch;
    *(dest + 1) = pitch;
}

extern "C" int phd_TranslateRel(sint32 x, sint32 y, sint32 z)
{
    sint32 *mptr = phd_mxptr;

    *(mptr + M03) += *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
    *(mptr + M13) += *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
    *(mptr + M23) += *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;
    if ((x = ABS(*(mptr + M03))) > phd_zfar || (y = ABS(*(mptr + M13))) > phd_zfar || (z = ABS(*(mptr + M23))) > phd_zfar)
        return 0;
    return 1;
}

extern "C" sint16 *calc_object_vertices(sint16 *objptr);
extern "C" sint16 *calc_vertice_light(sint16 *objptr);
extern "C" sint16 *(*InsertObjectG4)(sint16 *objptr, int number, enum sort_type sort_routine);
extern "C" sint16 *(*InsertObjectG3)(sint16 *objptr, int number, enum sort_type sort_routine);

extern "C" void phd_PutPolygons(sint16 *objptr, int clipstatus)
{
    phd_leftfloat = (float)phd_winxmin;
    phd_topfloat = (float)phd_winymin;
    phd_rightfloat = (float)(phd_winxmin + phd_winxmax + 1);
    phd_bottomfloat = (float)(phd_winymin + phd_winymax + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);
    objptr += 4;
    objptr = calc_object_vertices(objptr);
    if (objptr)
    {
        objptr = calc_vertice_light(objptr);
        objptr = InsertObjectGT4(objptr + 1, *(objptr), MID_SORT);
        objptr = InsertObjectGT3(objptr + 1, *(objptr), MID_SORT);
        objptr = InsertObjectG4(objptr + 1, *(objptr), MID_SORT);
        objptr = InsertObjectG3(objptr + 1, *(objptr), MID_SORT);
    }
}

void S_InsertRoom(sint16 *objptr, int outside)
{
    phd_leftfloat = (float)(phd_winxmin + phd_left);
    phd_topfloat = (float)(phd_winymin + phd_top);
    phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
    phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);
    objptr = calc_roomvert(objptr, outside ? 0 : 16);
    objptr = InsertObjectGT4(objptr + 1, *(objptr), FAR_SORT);
    objptr = InsertObjectGT3(objptr + 1, *(objptr), FAR_SORT);
    objptr = ins_room_sprite(objptr + 1, *(objptr));
}

void phd_RotateLight(PHD_ANGLE pitch, PHD_ANGLE yaw)
{
    sint32 cx;
    PHD_VECTOR ls_world;
    sint32 *mptr;

    phd_ls_yaw = yaw;
    phd_ls_pitch = pitch;
    cx = (sint32)phd_cos(pitch);
    ls_world.x = TRIGMULT2(cx, (sint32)phd_sin(yaw));
    ls_world.y = -phd_sin(pitch);
    ls_world.z = TRIGMULT2(cx, (sint32)phd_cos(yaw));
    mptr = w2v_matrix;
    ls_vector_view.x = (*(mptr + M00) * ls_world.x + *(mptr + M01) * ls_world.y + *(mptr + M02) * ls_world.z) >> W2V_SHIFT;
    ls_vector_view.y = (*(mptr + M10) * ls_world.x + *(mptr + M11) * ls_world.y + *(mptr + M12) * ls_world.z) >> W2V_SHIFT;
    ls_vector_view.z = (*(mptr + M20) * ls_world.x + *(mptr + M21) * ls_world.y + *(mptr + M22) * ls_world.z) >> W2V_SHIFT;
}

extern "C" sint32 *sort3dptr;
extern "C" sint16 *info3dptr;
extern "C" sint16 info3d_buffer[];
void HWR_InitVertexList();

extern "C" void phd_InitPolyList(void)
{
    surfacenum = 0;
    sort3dptr = (sint32 *)sort3d_buffer;
    info3dptr = info3d_buffer;
    if (SavedAppSettings.RenderMode != RM_Software)
        HWR_InitVertexList();
}

extern "C" void do_quickysorty(int left, int right);

extern "C" void phd_SortPolyList(void)
{
    sint32 i;

    if (surfacenum)
    {
        for (i = 0; i < surfacenum; i++)
            sort3d_buffer[i][1] += i;

        do_quickysorty(0, surfacenum - 1);
    }
}

extern "C" void do_quickysorty(int left, int right)
{
    sint32 i, j;
    sint32 compare, swap;

    i = left;
    j = right;
    compare = sort3d_buffer[(left + right) / 2][1];

    do
    {
        while (sort3d_buffer[i][1] > compare && i < right)
            ++i;
        while (compare > sort3d_buffer[j][1] && j > left)
            --j;
        if (i <= j)
        {
            swap = sort3d_buffer[i][1];
            sort3d_buffer[i][1] = sort3d_buffer[j][1];
            sort3d_buffer[j][1] = swap;

            swap = sort3d_buffer[i][0];
            sort3d_buffer[i][0] = sort3d_buffer[j][0];
            sort3d_buffer[j][0] = swap;

            i++;
            j--;
        }
    }
    while (i <= j);

    if (left < j)
        do_quickysorty(left, j);
    if (i < right)
        do_quickysorty(i, right);
}

void phd_PrintPolyList(void *ptr)
{
    int i;
    sint32 *sptr;
    sint16 *iptr;
    sint16 routine;

    phd_winptr = (char *)ptr;
    sptr = (sint32 *)sort3d_buffer;
    for (i = surfacenum; i > 0; i--) {
        iptr = (sint16 *)(*sptr);
        routine = *(iptr++);
        (*poly_draw_routines[routine])(iptr);
        sptr += 2;
    }
}

extern "C" float FltNearZ;
extern "C" float FltFarZ;
extern "C" float FltResZ;
extern "C" float FltResZORhw;
extern "C" float FltResZBuf;
extern "C" float RhwFactor;

extern "C" float FltRhwONearZ;
extern "C" float FltPerspONearZ;
extern "C" float FltPersp;
extern "C" float FltRhwOPersp;
extern "C" sint32 PhdPersp;

typedef struct {
    APP_SETTINGS base;
    bool BilinearFiltering;
    bool TripleBuffering;
    bool FullScreen;
} APP_SETTINGS_EXT;

extern "C" void AlterFOV(PHD_ANGLE fov)
{
    float aspect, ratio;

    fov /= 2;
    PhdPersp = ((phd_winwidth / 2) * phd_cos(fov)) / phd_sin(fov);
    FltPersp = (float)PhdPersp;
    FltRhwOPersp = RhwFactor / FltPersp;
    FltPerspONearZ = FltPersp / FltNearZ;
    ratio = (float)phd_winwidth / (float)phd_winheight;
    aspect = 4.0f / 3.0f;
    if (!((APP_SETTINGS_EXT *)&SavedAppSettings)->FullScreen && SavedAppSettings.AspectMode == 1)
        aspect = 16.0f / 9.0f;
    LfAspectCorrection = aspect / ratio;
}

extern "C" void Func402620(void);

extern "C" void phd_SetNearZ(sint32 z)
{
    phd_znear = z;
    FltNearZ = (float)z;
    FltRhwONearZ = RhwFactor / FltNearZ;
    FltPerspONearZ = FltPersp / FltNearZ;
    Func402620();
}

extern "C" void phd_SetFarZ(sint32 z)
{
    phd_zfar = z;
    FltFarZ = (float)z;
    Func402620();
}

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint16 clip, g, u, v;
} PHD_VBUF;

extern "C" PHD_VBUF PhdVBuf[];

extern "C" sint16 *calc_background_light(sint16 *objptr)
{
    int numvert, i;

    numvert = *(objptr++);
    if (numvert > 0)
        objptr += numvert * 3;
    else if (numvert < 0) {
        numvert = -numvert;
        objptr += numvert;
    }

    for (i = 0; i < numvert; i++)
        PhdVBuf[i].g = 0xfff;

    return objptr;
}
