#include "../3dglodef.h"

extern "C" sint32 *sort3dptr;
extern "C" sint16 *info3dptr;

typedef struct vertex_info {
    float x, y;
    float ooz;
    float u, v, g;
} VERTEX_INFO;

typedef struct point_info {
    float xv, yv, zv;
    float ooz;
    float xs, ys;
    float u, v;
    float g;
} POINT_INFO;

extern "C" float FltNearZ;
extern "C" float FltRhwONearZ;
extern "C" float FltPerspONearZ;

extern "C" int ZedClipper(int number, POINT_INFO *input, VERTEX_INFO *output)
{
    int nVertices;
    POINT_INFO *A, *B;
    VERTEX_INFO *pOut;
    float clipper, fADiff, fBDiff;

    A = input + number - 1;
    B = input;
    pOut = output;
    nVertices = 0;
    for (; number--; A = B++) {
        fADiff = FltNearZ - A->zv;
        fBDiff = FltNearZ - B->zv;
        if (((*(long *)&fADiff) | (*(long *)&fBDiff)) < 0) {
            if (((*(long *)&fADiff) ^ (*(long *)&fBDiff)) < 0) {
                clipper = fBDiff / (A->zv - B->zv);
                pOut->x = (B->xv + (A->xv - B->xv) * clipper) * FltPerspONearZ + f_centerx;
                pOut->y = (B->yv + (A->yv - B->yv) * clipper) * FltPerspONearZ + f_centery;
                pOut->g = B->g + (A->g - B->g) * clipper;
                pOut->ooz = FltRhwONearZ;
                pOut->u = (B->u + (A->u - B->u) * clipper) * FltRhwONearZ;
                pOut->v = (B->v + (A->v - B->v) * clipper) * FltRhwONearZ;
                ++pOut;
                ++nVertices;
            }
            if ((*(long *)&fBDiff) < 0) {
                pOut->x = B->xs;
                pOut->y = B->ys;
                pOut->g = B->g;
                pOut->ooz = B->ooz;
                pOut->u = B->u * B->ooz;
                pOut->v = B->v * B->ooz;
                ++pOut;
                ++nVertices;
            }
        }
    }
    if (nVertices < 3)
        return 0;
    return nVertices;
}

#define CLIP_GUV(OUT, B, A) \
    OUT.ooz = B->ooz + (A->ooz - B->ooz) * clipper; \
    OUT.u = B->u + (A->u - B->u) * clipper; \
    OUT.v = B->v + (A->v - B->v) * clipper; \
    OUT.g = B->g + (A->g - B->g) * clipper

static VERTEX_INFO XYGUV_output[8];

extern "C" int XYGUVClipper(int number, VERTEX_INFO *input)
{
    VERTEX_INFO *A, *B;
    int i, j;
    float clipper;

    B = &input[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &input[i];
        if (A->x < phd_leftfloat) {
            if (B->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            CLIP_GUV(XYGUV_output[j], B, A);
            XYGUV_output[j].x = phd_leftfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (A->x > phd_rightfloat) {
            if (B->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            CLIP_GUV(XYGUV_output[j], B, A);
            XYGUV_output[j].x = phd_rightfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        }
        if (B->x < phd_leftfloat) {
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            CLIP_GUV(XYGUV_output[j], B, A);
            XYGUV_output[j].x = phd_leftfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (B->x > phd_rightfloat) {
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            CLIP_GUV(XYGUV_output[j], B, A);
            XYGUV_output[j].x = phd_rightfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else {
            XYGUV_output[j].x = B->x;
            XYGUV_output[j].y = B->y;
            XYGUV_output[j].g = B->g;
            XYGUV_output[j].ooz = B->ooz;
            XYGUV_output[j].u = B->u;
            XYGUV_output[j++].v = B->v;
        }
    }
    if (j < 3)
        return 0;
    number = j;
    B = &XYGUV_output[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &XYGUV_output[i];
        if (A->y < phd_topfloat) {
            if (B->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            CLIP_GUV(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (A->y > phd_bottomfloat) {
            if (B->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            CLIP_GUV(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        }
        if (B->y < phd_topfloat) {
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            CLIP_GUV(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (B->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            CLIP_GUV(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        } else {
            input[j].x = B->x;
            input[j].y = B->y;
            input[j].g = B->g;
            input[j].ooz = B->ooz;
            input[j].u = B->u;
            input[j++].v = B->v;
        }
    }
    if (j < 3)
        return 0;
    return j;
}

#define CLIP_G(OUT, B, A) \
    { \
        OUT.ooz = B->ooz + (A->ooz - B->ooz) * clipper; \
        OUT.g = B->g + (A->g - B->g) * clipper; \
    }

static VERTEX_INFO XYG_output[8];

extern "C" int XYGClipper(int number, VERTEX_INFO *input)
{
    int i, j;
    VERTEX_INFO *A, *B;
    float clipper;

    B = &input[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &input[i];
        if (A->x < phd_leftfloat) {
            if (B->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            CLIP_G(XYG_output[j], B, A);
            XYG_output[j].x = phd_leftfloat;
            XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (A->x > phd_rightfloat) {
            if (B->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            CLIP_G(XYG_output[j], B, A);
            XYG_output[j].x = phd_rightfloat;
            XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
        }
        if (B->x < phd_leftfloat) {
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            CLIP_G(XYG_output[j], B, A);
            XYG_output[j].x = phd_leftfloat;
            XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (B->x > phd_rightfloat) {
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            CLIP_G(XYG_output[j], B, A);
            XYG_output[j].x = phd_rightfloat;
            XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else {
            XYG_output[j].x = B->x;
            XYG_output[j].y = B->y;
            XYG_output[j].ooz = B->ooz;
            XYG_output[j].g = B->g;
            j++;
        }
    }
    if (j < 3)
        return 0;
    number = j;
    B = &XYG_output[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &XYG_output[i];
        if (A->y < phd_topfloat) {
            if (B->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            CLIP_G(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (A->y > phd_bottomfloat) {
            if (B->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            CLIP_G(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        }
        if (B->y < phd_topfloat) {
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            CLIP_G(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (B->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            CLIP_G(input[j], B, A);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        } else {
            input[j].x = B->x;
            input[j].y = B->y;
            input[j].ooz = B->ooz;
            input[j].g = B->g;
            j++;
        }
    }
    if (j < 3)
        return 0;
    return j;
}

extern "C" int XYClipper(int number, VERTEX_INFO *input)
{
    int i, j;
    VERTEX_INFO output[20], *A, *B;
    float clipper;

    B = &input[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &input[i];
        if (A->x < phd_leftfloat) {
            if (B->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            output[j].x = phd_leftfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (A->x > phd_rightfloat) {
            if (B->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            output[j].x = phd_rightfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        }
        if (B->x < phd_leftfloat) {
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            output[j].x = phd_leftfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (B->x > phd_rightfloat) {
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            output[j].x = phd_rightfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else {
            output[j].x = B->x;
            output[j++].y = B->y;
        }
    }
    if (j < 3)
        return 0;
    number = j;
    B = &output[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &output[i];
        if (A->y < phd_topfloat) {
            if (B->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (A->y > phd_bottomfloat) {
            if (B->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        }
        if (B->y < phd_topfloat) {
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (B->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        } else {
            input[j].x = B->x;
            input[j++].y = B->y;
        }
    }
    if (j < 3)
        return 0;
    return j;
}

extern "C" void ins_trans_quad(sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 z)
{
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = phd_znear + (z << 3);
    sort3dptr += 2;
    info3dptr[0] = 7;
    info3dptr[1] = 32;
    info3dptr[2] = 4;
    info3dptr[3] = (sint16)sx;
    info3dptr[4] = (sint16)sy;
    info3dptr[5] = (sint16)(sx + w);
    info3dptr[6] = (sint16)sy;
    info3dptr[7] = (sint16)(sx + w);
    info3dptr[8] = (sint16)(sy + h);
    info3dptr[9] = (sint16)sx;
    info3dptr[10] = (sint16)(sy + h);
    info3dptr += 11;
    surfacenum++;
}

extern "C" void ins_flat_rect(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int z, int colour)
{
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 5;
    info3dptr[1] = (sint16)colour;
    info3dptr[2] = 4;
    info3dptr[3] = (sint16)min_x;
    info3dptr[4] = (sint16)min_y;
    info3dptr[5] = (sint16)max_x;
    info3dptr[6] = (sint16)min_y;
    info3dptr[7] = (sint16)max_x;
    info3dptr[8] = (sint16)max_y;
    info3dptr[9] = (sint16)min_x;
    info3dptr[10] = (sint16)max_y;
    info3dptr += 11;
    surfacenum++;
}

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
