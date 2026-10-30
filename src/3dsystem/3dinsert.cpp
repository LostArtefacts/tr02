#include "../3dglodef.h"

extern "C" sint32 *sort3dptr;
extern "C" sint16 *info3dptr;

extern "C" void ins_line(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color)
{
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 4;
    info3dptr[1] = (sint16)(x1 + phd_winxmin);
    info3dptr[2] = (sint16)(y1 + phd_winymin);
    info3dptr[3] = (sint16)(x2 + phd_winxmin);
    info3dptr[4] = (sint16)(y2 + phd_winymin);
    info3dptr[5] = (sint16)color;
    info3dptr += 6;
    surfacenum++;
}
