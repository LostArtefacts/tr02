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
