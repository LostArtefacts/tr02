#include "../specific/global.h"
#include "../lara.h"
#include "../specific/hwrender.h"
#include "../specific/output.h"
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

extern "C" void phd_RotZ(PHD_ANGLE rz)
{
    sint32 sz, cz, r0, r1;
    sint32 *mptr;

    if (rz)
    {
        mptr = phd_mxptr;
        sz = phd_sin(rz);
        cz = phd_cos(rz);
        r0 = *(mptr + M00) * cz + *(mptr + M01) * sz;
        r1 = *(mptr + M01) * cz - *(mptr + M00) * sz;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M01) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M10) * cz + *(mptr + M11) * sz;
        r1 = *(mptr + M11) * cz - *(mptr + M10) * sz;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M11) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M20) * cz + *(mptr + M21) * sz;
        r1 = *(mptr + M21) * cz - *(mptr + M20) * sz;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M21) = r1 >> W2V_SHIFT;
    }
}

extern "C" void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz)
{
    sint32 sina, cosa, r0, r1;
    sint32 *mptr;

    mptr = phd_mxptr;
    if (ry)
    {
        sina = phd_sin(ry);
        cosa = phd_cos(ry);
        r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    if (rx)
    {
        sina = phd_sin(rx);
        cosa = phd_cos(rx);
        r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
        *(mptr + M01) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
        *(mptr + M11) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
        *(mptr + M21) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    if (rz)
    {
        sina = phd_sin(rz);
        cosa = phd_cos(rz);
        r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
        r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M01) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
        r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M11) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
        r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M21) = r1 >> W2V_SHIFT;
    }
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

void phd_TranslateAbs(sint32 x, sint32 y, sint32 z)
{
    sint32 *mptr = phd_mxptr;

    x -= *(w2v_matrix + M03);
    y -= *(w2v_matrix + M13);
    z -= *(w2v_matrix + M23);
    *(mptr + M03) = *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
    *(mptr + M13) = *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
    *(mptr + M23) = *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;
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

void HWR_EnableZBuffer(bool tWrite, bool tCompare);
extern "C" sint16 *calc_background_light(sint16 *objptr);

extern "C" void S_InsertBackground(sint16 *objptr)
{
    phd_leftfloat = (float)(phd_winxmin + phd_left);
    phd_topfloat = (float)(phd_winymin + phd_top);
    phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
    phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);
    objptr = calc_object_vertices(objptr + 4);
    if (objptr) {
        if (SavedAppSettings.RenderMode == RM_Hardware)
            HWR_EnableZBuffer(false, false);
        objptr = calc_background_light(objptr);
        objptr = InsertObjectGT4(objptr + 1, *(objptr), BACK_SORT);
        objptr = InsertObjectGT3(objptr + 1, *(objptr), BACK_SORT);
        objptr = InsertObjectG4(objptr + 1, *(objptr), BACK_SORT);
        objptr = InsertObjectG3(objptr + 1, *(objptr), BACK_SORT);
        if (SavedAppSettings.RenderMode == RM_Hardware)
            HWR_EnableZBuffer(true, true);
    }
}

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint16 clip, g, u, v;
} PHD_VBUF;

extern "C" PHD_VBUF PhdVBuf[];
extern "C" sint32 ZBufferSurface;
extern "C" sint32 MidSort;
extern "C" sint32 WibbleOffset;
extern "C" float FltNearZ;
extern "C" float FltFarZ;
extern "C" float FltPersp;
extern "C" float FltRhwOPersp;
extern "C" float Data478044;

extern "C" sint16 *calc_roomvert(sint16 *objptr, int far_clip)
{
    PHD_VBUF *v;
    sint32 *mptr;
    int numvert, zv_int, g;
    float xv, yv, zv, persp, base_z, xs, ys;
    sint8 clip;

    if (ZBufferSurface)
        base_z = 0.0f;
    else
        base_z = (float)(MidSort << 22);
    mptr = phd_mxptr;
    numvert = *(objptr++);
    for (v = PhdVBuf; numvert > 0; numvert--, v++, objptr += 6) {
        v->xv = xv = (float)(mptr[0] * objptr[0] + mptr[1] * objptr[1] + mptr[2] * objptr[2] + mptr[3]);
        v->yv = yv = (float)(mptr[4] * objptr[0] + mptr[5] * objptr[1] + mptr[6] * objptr[2] + mptr[7]);
        zv_int = mptr[8] * objptr[0] + mptr[9] * objptr[1] + mptr[10] * objptr[2] + mptr[11];
        v->zv = zv = (float)zv_int;
        g = objptr[5];
        if (shade_effect)
            g += shade_table[(rand_table[numvert & 0x1f] + WibbleOffset) & 0x1f];
        if (zv >= FltNearZ) {
            persp = FltPersp / zv;
            clip = 0;
            v->zv += base_z;
            if (zv_int >= 0x14000000) {
                clip = far_clip;
                v->g = 0x1fff;
                v->zv = FltFarZ;
                v->ooz = Data478044;
            } else {
                if (zv_int > 0xc000000)
                    v->g = (zv_int >> W2V_SHIFT) + g - 0x3000;
                else
                    v->g = g;
                v->ooz = persp * FltRhwOPersp;
            }
            xs = xv * persp + f_centerx;
            ys = yv * persp + f_centery;
            if (water_effect && objptr[4] >= 0) {
                xs += wibble_table[(WibbleOffset + (uint8)ys) & 0x1f];
                ys += wibble_table[(WibbleOffset + (uint8)xs) & 0x1f];
            }
            if (xs < phd_leftfloat)
                clip++;
            else if (xs > phd_rightfloat)
                clip += 2;
            v->xs = xs;
            if (ys < phd_topfloat)
                clip += 4;
            else if (ys > phd_bottomfloat)
                clip += 8;
            ((uint8 *)&v->clip)[0] = clip;
            v->ys = ys;
            ((uint8 *)&v->clip)[1] = ~(uint8)(int)(v->zv * 0.000000715255737304688f);
        } else {
            ((uint8 *)&v->clip)[0] = 0x80;
            v->g = g;
            ((uint8 *)&v->clip)[1] = 0xff;
        }
        if (v->g < 0)
            v->g = 0;
        else if (v->g > 0x1fff)
            v->g = 0x1fff;
    }
    return objptr;
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

extern "C" {
extern sint32 phd_scrwidth;
extern sint32 PhdScreenHeight;
extern sint32 PhdViewDistance;
extern void (*InsertSprite)(int z, int x1, int y1, int x2, int y2, int sprnum, int shade);
extern void (*InsertTrans8)(void *vbuf, sint16 shade);
extern void (*InsertTransQuad)(sint32 x, sint32 y, sint32 w, sint32 h, sint32 z);
sint16 *ins_objectGT3(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *ins_objectGT4(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *ins_objectG3(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *ins_objectG4(sint16 *objptr, int number, enum sort_type sort_routine);
void insert_sprite(int z, int x1, int y1, int x2, int y2, int sprnum, int shade);
void ins_flat_rect(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);
void ins_line(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
void ins_poly_trans8(void *vbuf, sint16 shade);
void ins_trans_quad(sint32 x, sint32 y, sint32 w, sint32 h, sint32 z);
sint16 *InsertObjectGT3_ZBuffered(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectGT4_ZBuffered(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectG3_ZBuffered(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectG4_ZBuffered(sint16 *objptr, int number, enum sort_type sort_routine);
void InsertFlatRect_ZBuffered(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);
void InsertLine_ZBuffered(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
sint16 *InsertObjectGT3_Sorted(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectGT4_Sorted(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectG3_Sorted(sint16 *objptr, int number, enum sort_type sort_routine);
sint16 *InsertObjectG4_Sorted(sint16 *objptr, int number, enum sort_type sort_routine);
void InsertFlatRect_Sorted(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);
void InsertLine_Sorted(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
void InsertSprite_Sorted(int z, int x1, int y1, int x2, int y2, int sprnum, int shade);
void InsertTrans8_Sorted(void *vbuf, sint16 shade);
void InsertTransQuad_Sorted(sint32 x, sint32 y, sint32 w, sint32 h, sint32 z);
}

extern "C" void phd_InitWindow(sint16 x, sint16 y, int width, int height, int nearz, int farz, sint16 view_angle, int scrwidth, int scrheight)
{
    phd_winxmin = x;
    phd_winymin = y;
    phd_winxmax = width - 1;
    phd_winymax = height - 1;
    phd_winwidth = width;
    phd_winheight = height;
    phd_centerx = width / 2;
    phd_centery = height / 2;
    phd_znear = nearz << W2V_SHIFT;
    phd_zfar = farz << W2V_SHIFT;
    PhdViewDistance = farz;
    phd_scrwidth = scrwidth;
    PhdScreenHeight = scrheight;
    f_centerx = (float)phd_centerx;
    f_centery = (float)phd_centery;
    AlterFOV(view_angle * ONE_DEGREE);
    phd_SetNearZ(phd_znear);
    phd_SetFarZ(phd_zfar);
    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;
    phd_mxptr = matrix_stack;
    GrcDumpWindow.left = phd_winxmin;
    GrcDumpWindow.top = phd_winymin;
    GrcDumpWindow.right = phd_winxmin + phd_winwidth;
    GrcDumpWindow.bottom = phd_winymin + phd_winheight;
    switch (SavedAppSettings.RenderMode) {
    case RM_Software:
        InsertObjectGT3 = ins_objectGT3;
        InsertObjectGT4 = ins_objectGT4;
        InsertObjectG3 = ins_objectG3;
        InsertObjectG4 = ins_objectG4;
        InsertSprite = insert_sprite;
        InsertFlatRect = ins_flat_rect;
        InsertLine = ins_line;
        InsertTrans8 = ins_poly_trans8;
        InsertTransQuad = ins_trans_quad;
        perspective_distance = SavedAppSettings.PerspectiveCorrect ? 0x6000000 : 0x3000000;
        break;
    case RM_Hardware:
        if (SavedAppSettings.ZBuffer) {
            InsertObjectGT3 = InsertObjectGT3_ZBuffered;
            InsertObjectGT4 = InsertObjectGT4_ZBuffered;
            InsertObjectG3 = InsertObjectG3_ZBuffered;
            InsertObjectG4 = InsertObjectG4_ZBuffered;
            InsertFlatRect = InsertFlatRect_ZBuffered;
            InsertLine = InsertLine_ZBuffered;
        } else {
            InsertObjectGT3 = InsertObjectGT3_Sorted;
            InsertObjectGT4 = InsertObjectGT4_Sorted;
            InsertObjectG3 = InsertObjectG3_Sorted;
            InsertObjectG4 = InsertObjectG4_Sorted;
            InsertFlatRect = InsertFlatRect_Sorted;
            InsertLine = InsertLine_Sorted;
        }
        InsertSprite = InsertSprite_Sorted;
        InsertTrans8 = InsertTrans8_Sorted;
        InsertTransQuad = InsertTransQuad_Sorted;
        break;
    }
}

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
