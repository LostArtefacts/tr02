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
