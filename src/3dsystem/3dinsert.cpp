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

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint16 clip, g, u, v;
} PHD_VBUF;

typedef struct {
    sint16 xs, ys, g;
    float ooz, u, v;
} OUTPUT_INFO;

extern "C" PHD_VBUF PhdVBuf[];
extern "C" VERTEX_INFO VBuffer[];
extern "C" int visible_zclip(PHD_VBUF *vn1, PHD_VBUF *vn2, PHD_VBUF *vn3);

extern "C" sint16 *ins_objectGT4(sint16 *objptr, int number, enum sort_type sort_routine)
{
    PHD_VBUF *vn1, *vn2, *vn3, *vn4;
    PHDTEXTURESTRUCT *tex;
    VERTEX_INFO *v;
    POINT_INFO point[4], *p;
    OUTPUT_INFO *outptr;
    int i, npoints;
    float zdepth;

    for (; number > 0; number--, objptr += 5) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        vn4 = &PhdVBuf[objptr[3]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip & vn4->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            tex = &phdtextinfo[objptr[4]];
            p = point;
            p->xv = vn1->xv;
            p->yv = vn1->yv;
            p->zv = vn1->zv;
            p->ooz = vn1->ooz;
            p->xs = vn1->xs;
            p->ys = vn1->ys;
            p->g = (float)vn1->g;
            p->u = (float)tex->u1;
            p->v = (float)tex->v1;
            p++;
            p->xv = vn2->xv;
            p->yv = vn2->yv;
            p->zv = vn2->zv;
            p->ooz = vn2->ooz;
            p->xs = vn2->xs;
            p->ys = vn2->ys;
            p->g = (float)vn2->g;
            p->u = (float)tex->u2;
            p->v = (float)tex->v2;
            p++;
            p->xv = vn3->xv;
            p->yv = vn3->yv;
            p->zv = vn3->zv;
            p->ooz = vn3->ooz;
            p->xs = vn3->xs;
            p->ys = vn3->ys;
            p->g = (float)vn3->g;
            p->u = (float)tex->u3;
            p->v = (float)tex->v3;
            p++;
            p->xv = vn4->xv;
            p->yv = vn4->yv;
            p->zv = vn4->zv;
            p->ooz = vn4->ooz;
            p->xs = vn4->xs;
            p->ys = vn4->ys;
            p->g = (float)vn4->g;
            p->u = (float)tex->u4;
            p->v = (float)tex->v4;
            npoints = ZedClipper(4, point, VBuffer);
            if (npoints)
                npoints = XYGUVClipper(npoints, VBuffer);
        } else if ((sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip)) {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            tex = &phdtextinfo[objptr[4]];
            v = VBuffer;
            v->x = vn1->xs;
            v->y = vn1->ys;
            v->ooz = vn1->ooz;
            v->g = (float)vn1->g;
            v->u = tex->u1 * v->ooz;
            v->v = tex->v1 * v->ooz;
            v++;
            v->x = vn2->xs;
            v->y = vn2->ys;
            v->ooz = vn2->ooz;
            v->g = (float)vn2->g;
            v->u = tex->u2 * v->ooz;
            v->v = tex->v2 * v->ooz;
            v++;
            v->x = vn3->xs;
            v->y = vn3->ys;
            v->ooz = vn3->ooz;
            v->g = (float)vn3->g;
            v->u = tex->u3 * v->ooz;
            v->v = tex->v3 * v->ooz;
            v++;
            v->x = vn4->xs;
            v->y = vn4->ys;
            v->ooz = vn4->ooz;
            v->g = (float)vn4->g;
            v->u = tex->u4 * v->ooz;
            v->v = tex->v4 * v->ooz;
            npoints = XYGUVClipper(4, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            switch (sort_routine) {
            case FAR_SORT:
                zdepth = vn1->zv;
                if (vn2->zv > zdepth)
                    zdepth = vn2->zv;
                if (vn3->zv > zdepth)
                    zdepth = vn3->zv;
                if (vn4->zv > zdepth)
                    zdepth = vn4->zv;
                break;
            case MID_SORT:
                zdepth = (vn1->zv + vn2->zv + vn3->zv + vn4->zv) * 0.25f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            tex = &phdtextinfo[objptr[4]];
            if (zdepth < perspective_distance) {
                info3dptr[0] = tex->drawtype + 2;
                info3dptr[1] = tex->tpage;
                info3dptr[2] = 4;
                outptr = (OUTPUT_INFO *)(info3dptr + 3);
                outptr[0].xs = (sint16)vn1->xs;
                outptr[0].ys = (sint16)vn1->ys;
                outptr[0].g = vn1->g;
                outptr[0].ooz = vn1->ooz;
                outptr[0].u = tex->u1 * outptr[0].ooz;
                outptr[0].v = tex->v1 * outptr[0].ooz;
                outptr[1].xs = (sint16)vn2->xs;
                outptr[1].ys = (sint16)vn2->ys;
                outptr[1].g = vn2->g;
                outptr[1].ooz = vn2->ooz;
                outptr[1].u = tex->u2 * outptr[1].ooz;
                outptr[1].v = tex->v2 * outptr[1].ooz;
                outptr[2].xs = (sint16)vn3->xs;
                outptr[2].ys = (sint16)vn3->ys;
                outptr[2].g = vn3->g;
                outptr[2].ooz = vn3->ooz;
                outptr[2].u = tex->u3 * outptr[2].ooz;
                outptr[2].v = tex->v3 * outptr[2].ooz;
                outptr[3].xs = (sint16)vn4->xs;
                outptr[3].ys = (sint16)vn4->ys;
                outptr[3].g = vn4->g;
                outptr[3].ooz = vn4->ooz;
                outptr[3].u = tex->u4 * outptr[3].ooz;
                outptr[3].v = tex->v4 * outptr[3].ooz;
                info3dptr = (sint16 *)&outptr[4];
            } else {
                *(uint32 *)info3dptr = *(uint32 *)&tex->drawtype;
                info3dptr[2] = 4;
                info3dptr[3] = (sint16)vn1->xs;
                info3dptr[4] = (sint16)vn1->ys;
                info3dptr[5] = vn1->g;
                info3dptr[6] = tex->u1;
                info3dptr[7] = tex->v1;
                info3dptr[8] = (sint16)vn2->xs;
                info3dptr[9] = (sint16)vn2->ys;
                info3dptr[10] = vn2->g;
                info3dptr[11] = tex->u2;
                info3dptr[12] = tex->v2;
                info3dptr[13] = (sint16)vn3->xs;
                info3dptr[14] = (sint16)vn3->ys;
                info3dptr[15] = vn3->g;
                info3dptr[16] = tex->u3;
                info3dptr[17] = tex->v3;
                info3dptr[18] = (sint16)vn4->xs;
                info3dptr[19] = (sint16)vn4->ys;
                info3dptr[20] = vn4->g;
                info3dptr[21] = tex->u4;
                info3dptr[22] = tex->v4;
                info3dptr += 23;
            }
            surfacenum++;
            continue;
        }
        if (npoints) {
            switch (sort_routine) {
            case FAR_SORT:
                zdepth = vn1->zv;
                if (vn2->zv > zdepth)
                    zdepth = vn2->zv;
                if (vn3->zv > zdepth)
                    zdepth = vn3->zv;
                if (vn4->zv > zdepth)
                    zdepth = vn4->zv;
                break;
            case MID_SORT:
                zdepth = (vn1->zv + vn2->zv + vn3->zv + vn4->zv) * 0.25f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            if (zdepth < perspective_distance) {
                info3dptr[0] = tex->drawtype + 2;
                info3dptr[1] = tex->tpage;
                info3dptr[2] = npoints;
                outptr = (OUTPUT_INFO *)(info3dptr + 3);
                for (i = npoints, v = VBuffer; i > 0; i--, v++, outptr++) {
                    outptr->xs = (sint16)v->x;
                    outptr->ys = (sint16)v->y;
                    outptr->g = (sint16)v->g;
                    outptr->ooz = v->ooz;
                    outptr->u = v->u;
                    outptr->v = v->v;
                }
                info3dptr = (sint16 *)outptr;
            } else {
                *(uint32 *)info3dptr = *(uint32 *)&tex->drawtype;
                info3dptr[2] = npoints;
                info3dptr += 3;
                for (i = npoints, v = VBuffer; i > 0; i--, v++, info3dptr += 5) {
                    info3dptr[0] = (sint16)v->x;
                    info3dptr[1] = (sint16)v->y;
                    info3dptr[2] = (sint16)v->g;
                    info3dptr[3] = (sint16)(v->u / v->ooz);
                    info3dptr[4] = (sint16)(v->v / v->ooz);
                }
            }
            surfacenum++;
        }
    }
    return objptr;
}

extern "C" sint16 *ins_objectGT3(sint16 *objptr, int number, enum sort_type sort_routine)
{
    VERTEX_INFO *v;
    int npoints, i;
    POINT_INFO *p, point[3];
    float zdepth;
    PHD_VBUF *vn1, *vn2, *vn3;
    OUTPUT_INFO *outptr;
    PHDTEXTURESTRUCT *tex;

    for (; number > 0; number--, objptr += 4) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            tex = &phdtextinfo[objptr[3]];
            p = point;
            p->xv = vn1->xv;
            p->yv = vn1->yv;
            p->zv = vn1->zv;
            p->ooz = vn1->ooz;
            p->xs = vn1->xs;
            p->ys = vn1->ys;
            p->g = (float)vn1->g;
            p->u = (float)tex->u1;
            p->v = (float)tex->v1;
            p++;
            p->xv = vn2->xv;
            p->yv = vn2->yv;
            p->zv = vn2->zv;
            p->ooz = vn2->ooz;
            p->xs = vn2->xs;
            p->ys = vn2->ys;
            p->g = (float)vn2->g;
            p->u = (float)tex->u2;
            p->v = (float)tex->v2;
            p++;
            p->xv = vn3->xv;
            p->yv = vn3->yv;
            p->zv = vn3->zv;
            p->ooz = vn3->ooz;
            p->xs = vn3->xs;
            p->ys = vn3->ys;
            p->g = (float)vn3->g;
            p->u = (float)tex->u3;
            p->v = (float)tex->v3;
            npoints = ZedClipper(3, point, VBuffer);
            if (npoints)
                npoints = XYGUVClipper(npoints, VBuffer);
        } else if ((sint8)(vn1->clip | vn2->clip | vn3->clip)) {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            tex = &phdtextinfo[objptr[3]];
            v = VBuffer;
            v->x = vn1->xs;
            v->y = vn1->ys;
            v->ooz = vn1->ooz;
            v->g = (float)vn1->g;
            v->u = tex->u1 * v->ooz;
            v->v = tex->v1 * v->ooz;
            v++;
            v->x = vn2->xs;
            v->y = vn2->ys;
            v->ooz = vn2->ooz;
            v->g = (float)vn2->g;
            v->u = tex->u2 * v->ooz;
            v->v = tex->v2 * v->ooz;
            v++;
            v->x = vn3->xs;
            v->y = vn3->ys;
            v->ooz = vn3->ooz;
            v->g = (float)vn3->g;
            v->u = tex->u3 * v->ooz;
            v->v = tex->v3 * v->ooz;
            npoints = XYGUVClipper(3, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            switch (sort_routine) {
            case FAR_SORT:
                zdepth = vn1->zv;
                if (zdepth < vn2->zv)
                    zdepth = vn2->zv;
                if (zdepth < vn3->zv)
                    zdepth = vn3->zv;
                break;
            case MID_SORT:
                zdepth = (vn1->zv + vn2->zv + vn3->zv) * 0.333333333333333333f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            tex = &phdtextinfo[objptr[3]];
            if (zdepth < perspective_distance) {
                info3dptr[0] = tex->drawtype + 2;
                info3dptr[1] = tex->tpage;
                info3dptr[2] = 3;
                outptr = (OUTPUT_INFO *)(info3dptr + 3);
                outptr[0].xs = (sint16)vn1->xs;
                outptr[0].ys = (sint16)vn1->ys;
                outptr[0].g = vn1->g;
                outptr[0].ooz = vn1->ooz;
                outptr[0].u = tex->u1 * outptr[0].ooz;
                outptr[0].v = tex->v1 * outptr[0].ooz;
                outptr[1].xs = (sint16)vn2->xs;
                outptr[1].ys = (sint16)vn2->ys;
                outptr[1].g = vn2->g;
                outptr[1].ooz = vn2->ooz;
                outptr[1].u = tex->u2 * outptr[1].ooz;
                outptr[1].v = tex->v2 * outptr[1].ooz;
                outptr[2].xs = (sint16)vn3->xs;
                outptr[2].ys = (sint16)vn3->ys;
                outptr[2].g = vn3->g;
                outptr[2].ooz = vn3->ooz;
                outptr[2].u = tex->u3 * outptr[2].ooz;
                outptr[2].v = tex->v3 * outptr[2].ooz;
                info3dptr = (sint16 *)&outptr[3];
            } else {
                *(uint32 *)info3dptr = *(uint32 *)&tex->drawtype;
                info3dptr[2] = 3;
                info3dptr[3] = (sint16)vn1->xs;
                info3dptr[4] = (sint16)vn1->ys;
                info3dptr[5] = vn1->g;
                info3dptr[6] = tex->u1;
                info3dptr[7] = tex->v1;
                info3dptr[8] = (sint16)vn2->xs;
                info3dptr[9] = (sint16)vn2->ys;
                info3dptr[10] = vn2->g;
                info3dptr[11] = tex->u2;
                info3dptr[12] = tex->v2;
                info3dptr[13] = (sint16)vn3->xs;
                info3dptr[14] = (sint16)vn3->ys;
                info3dptr[15] = vn3->g;
                info3dptr[16] = tex->u3;
                info3dptr[17] = tex->v3;
                info3dptr += 18;
            }
            surfacenum++;
            continue;
        }
        if (npoints) {
            switch (sort_routine) {
            case FAR_SORT:
                zdepth = vn1->zv;
                if (zdepth < vn2->zv)
                    zdepth = vn2->zv;
                if (zdepth < vn3->zv)
                    zdepth = vn3->zv;
                break;
            case MID_SORT:
                zdepth = (vn1->zv + vn2->zv + vn3->zv) * 0.333333333333333333f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            if (zdepth < perspective_distance) {
                info3dptr[0] = tex->drawtype + 2;
                info3dptr[1] = tex->tpage;
                info3dptr[2] = npoints;
                outptr = (OUTPUT_INFO *)(info3dptr + 3);
                for (i = 0, v = VBuffer; i < npoints; i++, v++, outptr++) {
                    outptr->xs = (sint16)v->x;
                    outptr->ys = (sint16)v->y;
                    outptr->g = (sint16)v->g;
                    outptr->ooz = v->ooz;
                    outptr->u = v->u;
                    outptr->v = v->v;
                }
                info3dptr = (sint16 *)outptr;
            } else {
                *(uint32 *)info3dptr = *(uint32 *)&tex->drawtype;
                info3dptr[2] = npoints;
                info3dptr += 3;
                for (i = 0, v = VBuffer; i < npoints; i++, v++, info3dptr += 5) {
                    info3dptr[0] = (sint16)v->x;
                    info3dptr[1] = (sint16)v->y;
                    info3dptr[2] = (sint16)v->g;
                    info3dptr[3] = (sint16)(v->u / v->ooz);
                    info3dptr[4] = (sint16)(v->v / v->ooz);
                }
            }
            surfacenum++;
        }
    }
    return objptr;
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

extern "C" sint16 *ins_objectG4(sint16 *objptr, int number, enum sort_type sort_routine)
{
    VERTEX_INFO *v;
    int npoints, i;
    PHD_VBUF *vn1, *vn4, *vn2, *vn3;
    sint16 *outptr;
    float zdepth;
    POINT_INFO *p, point[4];

    outptr = info3dptr;
    for (; number > 0; number--, objptr += 5) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        vn4 = &PhdVBuf[objptr[3]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip & vn4->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            p = point;
            p->xv = vn1->xv;
            p->yv = vn1->yv;
            p->zv = vn1->zv;
            p->ooz = vn1->ooz;
            p->xs = vn1->xs;
            p->ys = vn1->ys;
            p->g = (float)vn1->g;
            p++;
            p->xv = vn2->xv;
            p->yv = vn2->yv;
            p->zv = vn2->zv;
            p->ooz = vn2->ooz;
            p->xs = vn2->xs;
            p->ys = vn2->ys;
            p->g = (float)vn2->g;
            p++;
            p->xv = vn3->xv;
            p->yv = vn3->yv;
            p->zv = vn3->zv;
            p->ooz = vn3->ooz;
            p->xs = vn3->xs;
            p->ys = vn3->ys;
            p->g = (float)vn3->g;
            p++;
            p->xv = vn4->xv;
            p->yv = vn4->yv;
            p->zv = vn4->zv;
            p->ooz = vn4->ooz;
            p->xs = vn4->xs;
            p->ys = vn4->ys;
            p->g = (float)vn4->g;
            npoints = ZedClipper(4, point, VBuffer);
            if (npoints)
                npoints = XYGClipper(npoints, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            v = VBuffer;
            v->x = vn1->xs;
            v->y = vn1->ys;
            v->g = (float)vn1->g;
            v++;
            v->x = vn2->xs;
            v->y = vn2->ys;
            v->g = (float)vn2->g;
            v++;
            v->x = vn3->xs;
            v->y = vn3->ys;
            v->g = (float)vn3->g;
            v++;
            v->x = vn4->xs;
            v->y = vn4->ys;
            v->g = (float)vn4->g;
            npoints = 4;
            if ((sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip))
                npoints = XYGClipper(npoints, VBuffer);
        }
        if (npoints) {
            if (sort_routine == MID_SORT)
                zdepth = (vn1->zv + vn2->zv + vn3->zv + vn4->zv) * 0.25f;
            else
                zdepth = 1000000000.0f;
            sort3dptr[0] = (sint32)outptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            outptr[0] = 6;
            outptr[1] = objptr[4] & 0xff;
            outptr[2] = npoints;
            outptr += 3;
            for (i = 0, v = VBuffer; i < npoints; i++, v++, outptr += 3) {
                outptr[0] = (sint16)v->x;
                outptr[1] = (sint16)v->y;
                outptr[2] = (sint16)v->g;
            }
            surfacenum++;
        }
    }
    info3dptr = outptr;
    return objptr;
}


extern "C" sint16 *ins_objectG3(sint16 *objptr, int number, enum sort_type sort_routine)
{
    PHD_VBUF *vn1, *vn2, *vn3;
    POINT_INFO *p, point[3];
    float zdepth;
    int npoints, i;
    VERTEX_INFO *v;
    sint16 *outptr;

    outptr = info3dptr;
    for (; number > 0; number--, objptr += 4) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            p = point;
            p->xv = vn1->xv;
            p->yv = vn1->yv;
            p->zv = vn1->zv;
            p->ooz = vn1->ooz;
            p->xs = vn1->xs;
            p->ys = vn1->ys;
            p->g = (float)vn1->g;
            p++;
            p->xv = vn2->xv;
            p->yv = vn2->yv;
            p->zv = vn2->zv;
            p->ooz = vn2->ooz;
            p->xs = vn2->xs;
            p->ys = vn2->ys;
            p->g = (float)vn2->g;
            p++;
            p->xv = vn3->xv;
            p->yv = vn3->yv;
            p->zv = vn3->zv;
            p->ooz = vn3->ooz;
            p->xs = vn3->xs;
            p->ys = vn3->ys;
            p->g = (float)vn3->g;
            npoints = ZedClipper(3, point, VBuffer);
            if (npoints)
                npoints = XYGClipper(npoints, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            v = VBuffer;
            v->x = vn1->xs;
            v->y = vn1->ys;
            v->g = (float)vn1->g;
            v++;
            v->x = vn2->xs;
            v->y = vn2->ys;
            v->g = (float)vn2->g;
            v++;
            v->x = vn3->xs;
            v->y = vn3->ys;
            v->g = (float)vn3->g;
            npoints = 3;
            if ((sint8)(vn1->clip | vn2->clip | vn3->clip))
                npoints = XYGClipper(npoints, VBuffer);
        }
        if (npoints) {
            if (sort_routine == MID_SORT)
                zdepth = (vn1->zv + vn2->zv + vn3->zv) * 0.333333333333333333f;
            else
                zdepth = 1000000000.0f;
            sort3dptr[0] = (sint32)outptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            outptr[0] = 6;
            outptr[1] = objptr[3] & 0xff;
            outptr[2] = npoints;
            outptr += 3;
            for (i = 0, v = VBuffer; i < npoints; i++, v++, outptr += 3) {
                outptr[0] = (sint16)v->x;
                outptr[1] = (sint16)v->y;
                outptr[2] = (sint16)v->g;
            }
            surfacenum++;
        }
    }
    info3dptr = outptr;
    return objptr;
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

extern "C" void ins_poly_trans8(PHD_VBUF *vbuf, sint16 shade)
{
    VERTEX_INFO *v;
    int npoints;
    PHD_VBUF *vn1, *vn5, *vn3, *vn4, *vn2, *vn6, *vn7, *vn8;

    vn1 = vbuf;
    vn2 = vbuf + 1;
    vn3 = vbuf + 2;
    vn4 = vbuf + 3;
    vn5 = vbuf + 4;
    vn6 = vbuf + 5;
    vn7 = vbuf + 6;
    vn8 = vbuf + 7;
    if ((sint8)(vn1->clip & vn2->clip & vn3->clip & vn4->clip & vn5->clip & vn6->clip & vn7->clip & vn8->clip)
        || (sint8)vn1->clip < 0 || (sint8)vn2->clip < 0 || (sint8)vn3->clip < 0 || (sint8)vn4->clip < 0
        || (sint8)vn5->clip < 0 || (sint8)vn6->clip < 0 || (sint8)vn7->clip < 0 || (sint8)vn8->clip < 0)
        return;
    if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
        return;
    if (!(sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip | vn5->clip | vn6->clip | vn7->clip | vn8->clip)) {
        sort3dptr[0] = (sint32)info3dptr;
        sort3dptr[1] = (sint32)((vn1->zv + vn2->zv + vn3->zv + vn4->zv + vn5->zv + vn6->zv + vn7->zv + vn8->zv) * 0.125f);
        sort3dptr += 2;
        info3dptr[0] = 7;
        info3dptr[1] = shade;
        info3dptr[2] = 8;
        info3dptr[3] = (sint16)vn1->xs;
        info3dptr[4] = (sint16)vn1->ys;
        info3dptr[5] = (sint16)vn2->xs;
        info3dptr[6] = (sint16)vn2->ys;
        info3dptr[7] = (sint16)vn3->xs;
        info3dptr[8] = (sint16)vn3->ys;
        info3dptr[9] = (sint16)vn4->xs;
        info3dptr[10] = (sint16)vn4->ys;
        info3dptr[11] = (sint16)vn5->xs;
        info3dptr[12] = (sint16)vn5->ys;
        info3dptr[13] = (sint16)vn6->xs;
        info3dptr[14] = (sint16)vn6->ys;
        info3dptr[15] = (sint16)vn7->xs;
        info3dptr[16] = (sint16)vn7->ys;
        info3dptr[17] = (sint16)vn8->xs;
        info3dptr[18] = (sint16)vn8->ys;
        info3dptr += 19;
        surfacenum++;
    } else {
        phd_leftfloat = phd_topfloat = 0.0f;
        phd_rightfloat = (float)phd_winxmax;
        phd_bottomfloat = (float)phd_winymax;
        v = VBuffer;
        v->x = vn1->xs;
        v->y = vn1->ys;
        v++;
        v->x = vn2->xs;
        v->y = vn2->ys;
        v++;
        v->x = vn3->xs;
        v->y = vn3->ys;
        v++;
        v->x = vn4->xs;
        v->y = vn4->ys;
        v++;
        v->x = vn5->xs;
        v->y = vn5->ys;
        v++;
        v->x = vn6->xs;
        v->y = vn6->ys;
        v++;
        v->x = vn7->xs;
        v->y = vn7->ys;
        v++;
        v->x = vn8->xs;
        v->y = vn8->ys;
        npoints = XYClipper(8, VBuffer);
        if (npoints) {
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)((vn1->zv + vn2->zv + vn3->zv + vn4->zv + vn5->zv + vn6->zv + vn7->zv + vn8->zv) * 0.125f);
            sort3dptr += 2;
            info3dptr[0] = 7;
            info3dptr[1] = shade;
            info3dptr[2] = npoints;
            info3dptr += 3;
            for (v = VBuffer; npoints > 0; npoints--, info3dptr += 2, v++) {
                info3dptr[0] = (sint16)v->x;
                info3dptr[1] = (sint16)v->y;
            }
            surfacenum++;
        }
    }
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
