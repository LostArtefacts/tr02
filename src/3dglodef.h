#ifndef _3DGLODEF_H
#define _3DGLODEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define WIBBLE_SIZE 32
#define MAX_WIBBLE 2
#define MAX_SHADE 0x300

#define W2V_SHIFT 14

#define ABS(x) (((x)<0) ? (-(x)):(x))

#define phd_PopMatrix() phd_mxptr -= 12

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

extern float wibble_table[];
extern sint32 rand_table[];
extern sint16 shade_table[];
extern sint32 *phd_mxptr;
extern sint32 phd_left;
extern sint32 phd_right;
extern sint32 phd_top;
extern sint32 phd_bottom;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern sint32 phd_winwidth;
extern sint32 phd_winheight;

void AlterFOV(PHD_ANGLE fov);

void phd_RotX(PHD_ANGLE rx);
void phd_RotY(PHD_ANGLE ry);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_PutPolygons(sint16 *objptr, int clipstatus);
void __cdecl phd_PushMatrix(void);

sint32 __fastcall phd_cos(sint32 angle);
sint32 __fastcall phd_sin(sint32 angle);
uint32 __fastcall phd_sqrt(uint32 n);
sint16 __fastcall phd_atan(sint32 x, sint32 y);

#ifdef __cplusplus
}
#endif

#endif
