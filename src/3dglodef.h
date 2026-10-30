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
#define W2V_SCALE (1 << W2V_SHIFT)

#define ABS(x) (((x)<0) ? (-(x)):(x))

#define TRIGMULT2(A, B) (((A) * (B)) >> W2V_SHIFT)
#define TRIGMULT3(A, B, C) (TRIGMULT2((TRIGMULT2(A, B)), C))

enum msoff {
    M00, M01, M02, M03,
    M10, M11, M12, M13,
    M20, M21, M22, M23
};

enum sort_type { MID_SORT, FAR_SORT, BACK_SORT };

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

typedef struct phdtexturestruct {
    uint16 drawtype;
    uint16 tpage;
    uint16 u1, v1;
    uint16 u2, v2;
    uint16 u3, v3;
    uint16 u4, v4;
} PHDTEXTURESTRUCT;

extern float wibble_table[];
extern sint32 water_effect;
extern sint32 shade_effect;
extern sint32 rand_table[];
extern sint16 shade_table[];
extern sint32 *phd_mxptr;
extern float phd_leftfloat;
extern float phd_rightfloat;
extern float phd_topfloat;
extern float phd_bottomfloat;
extern char *phd_winptr;
extern sint32 phd_centerx;
extern sint32 phd_centery;
extern float f_centerx;
extern float f_centery;
extern sint32 w2v_matrix[];
extern sint32 matrix_stack[];
extern PHD_VECTOR ls_vector_view;
extern PHD_ANGLE phd_ls_yaw, phd_ls_pitch;
extern sint32 surfacenum;
extern sint32 sort3d_buffer[][2];
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int perspective_distance;
extern sint32 phd_znear;
extern sint32 phd_zfar;
extern sint32 phd_left;
extern sint32 phd_right;
extern sint32 phd_top;
extern sint32 phd_bottom;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern sint32 phd_winwidth;
extern sint32 phd_winheight;

extern PHDTEXTURESTRUCT phdtextinfo[];
#ifdef __cplusplus
extern bool bBlueEffect;
#endif

extern void (*poly_draw_routines[])(sint16 *);
extern sint16 *(*InsertObjectGT3)(sint16 *objptr, int number, enum sort_type sort_routine);
extern sint16 *(*InsertObjectGT4)(sint16 *objptr, int number, enum sort_type sort_routine);
extern void (*InsertLine)(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
extern void (*InsertFlatRect)(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);

void AlterFOV(PHD_ANGLE fov);

void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void phd_RotX(PHD_ANGLE rx);
void phd_RotY(PHD_ANGLE ry);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_PutPolygons(sint16 *objptr, int clipstatus);
void S_InsertRoom(sint16 *objptr, int outside);
sint16 *calc_roomvert(sint16 *objptr, int far_clip);
void phd_RotateLight(PHD_ANGLE pitch, PHD_ANGLE yaw);
void phd_PrintPolyList(void *ptr);
sint16 *ins_room_sprite(sint16 *objptr, int num);
void __cdecl phd_PushMatrix(void);

sint32 __fastcall phd_cos(sint32 angle);
sint32 __fastcall phd_sin(sint32 angle);
uint32 __fastcall phd_sqrt(uint32 n);
sint16 __fastcall phd_atan(sint32 x, sint32 y);

#ifdef __cplusplus
}
#endif

#endif
