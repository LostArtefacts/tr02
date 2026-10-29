#ifndef _3DGLODEF_H
#define _3DGLODEF_H

#include "types.h"

#define ABS(x) (((x)<0) ? (-(x)):(x))

typedef sint16 PHD_ANGLE;

typedef struct phd_3dpos {
    sint32 x_pos;
    sint32 y_pos;
    sint32 z_pos;
    PHD_ANGLE x_rot;
    PHD_ANGLE y_rot;
    PHD_ANGLE z_rot;
} PHD_3DPOS;

void AlterFOV(PHD_ANGLE fov);

#endif
