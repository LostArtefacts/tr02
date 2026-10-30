#include "../3dglodef.h"

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
    PHD_VECTOR ls_world;
    sint32 *mptr;
    sint32 cx;

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
