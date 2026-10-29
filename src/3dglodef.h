#ifndef _3DGLODEF_H
#define _3DGLODEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define ABS(x) (((x)<0) ? (-(x)):(x))

typedef struct phd_vector {
    sint32 x;
    sint32 y;
    sint32 z;
} PHD_VECTOR;

typedef sint16 PHD_ANGLE;

typedef struct phd_3dpos {
    sint32 x_pos;
    sint32 y_pos;
    sint32 z_pos;
    PHD_ANGLE x_rot;
    PHD_ANGLE y_rot;
    PHD_ANGLE z_rot;
} PHD_3DPOS;

extern sint32 phd_winwidth;
extern sint32 phd_winheight;

void AlterFOV(PHD_ANGLE fov);

#ifdef __cplusplus
}
#endif

#endif
