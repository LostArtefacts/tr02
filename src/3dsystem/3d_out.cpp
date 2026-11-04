// cflags: /Op /Oa
#include <windows.h>
#include <process.h>
#include "../3dglodef.h"

typedef struct xbuf_x {
    sint32 Xleft;
    sint32 Xright;
} XBUF_X;

typedef struct xbuf_xguv {
    sint32 Xleft;
    sint32 Gleft;
    sint32 Uleft;
    sint32 Vleft;
    sint32 Xright;
    sint32 Gright;
    sint32 Uright;
    sint32 Vright;
} XBUF_XGUV;

typedef struct xbuf_persp {
    sint32 Xleft;
    sint32 Gleft;
    int UOZleft;
    int VOZleft;
    int OOZleft;
    sint32 Xright;
    sint32 Gright;
    int UOZright;
    int VOZright;
    int OOZright;
} XBUF_PERSP;

typedef struct x_info {
    sint16 xs;
    sint16 ys;
} X_INFO;

extern "C" XBUF_PERSP xbuffer[];

extern "C" void __fastcall flatA(int min_y, int max_y, char color);
extern "C" void __fastcall transA(int min_y, int max_y, char color);
extern "C" void draw_poly_trans(sint16 *iptr);
extern "C" void __fastcall gourA(int min_y, int max_y, sint16 colbase);
extern "C" int xgen_xg(sint16 *iptr);
extern "C" void draw_poly_gouraud(sint16 *iptr);
extern "C" char *texture_page_ptrs[];
extern "C" void __fastcall gtmapA(int min_y, int max_y, char *tptr);
extern "C" int xgen_xguv(sint16 *iptr);
extern "C" void draw_poly_gtmap(sint16 *iptr);
extern "C" void __fastcall wgtmapA(int min_y, int max_y, char *tptr);
extern "C" void draw_poly_wgtmap(sint16 *iptr);
extern "C" void draw_poly_gtmap_persp(sint16 *iptr);
extern "C" int xgen_xguvpersp_fp(sint16 *iptr);
extern "C" void gtmap_persp32_fp(int y1, int y2, unsigned char *tptr);
extern "C" void wgtmap_persp32_fp(int y1, int y2, unsigned char *tptr);
extern "C" void draw_poly_wgtmap_persp(sint16 *iptr);

static sint32 xgen_ymin;
static sint32 xgen_ymax;

void draw_poly_flat(sint16 *iptr)
{
    int colour;

    if (xgen_x(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call flatA
        }
    }
}

void draw_poly_trans(sint16 *iptr)
{
    int colour;

    if (xgen_x(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call transA
        }
    }
}

void draw_poly_gouraud(sint16 *iptr)
{
    int colour;

    if (xgen_xg(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call gourA
        }
    }
}

void draw_poly_gtmap(sint16 *iptr)
{
    char *texture;

    if (xgen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*(iptr)];
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call gtmapA
        }
    }
}

void draw_poly_wgtmap(sint16 *iptr)
{
    char *texture;

    if (xgen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*(iptr)];
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call wgtmapA
        }
    }
}

int xgen_x(sint16 *iptr)
{
    X_INFO *ptr1, *ptr2;
    XBUF_X *xptr;
    int x, x_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = (int)*(iptr++);
    ptr1 = (X_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0ffff;
            xptr = ((XBUF_X *)xbuffer) + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                xptr->Xright = x;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0001;
            xptr = ((XBUF_X *)xbuffer) + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                xptr->Xleft = x;
            }
        }
    }
    if (min_y == max_y)
        return (0);
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return (1);
}

typedef struct xg_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
} XG_INFO;

typedef struct xbuf_xg {
    sint32 Xleft;
    sint32 Gleft;
    sint32 Xright;
    sint32 Gright;
} XBUF_XG;

int xgen_xg(sint16 *iptr)
{
    XG_INFO *ptr1, *ptr2;
    XBUF_XG *xptr;
    int g, g_add;
    int x, x_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = (int)*(iptr++);
    ptr1 = (XG_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0ffff;
            g = ptr1->shade << 8;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            xptr = ((XBUF_XG *)xbuffer) + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                g += g_add;
                xptr->Xright = x;
                xptr->Gright = g;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0001;
            g = ptr2->shade << 8;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            xptr = ((XBUF_XG *)xbuffer) + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                g += g_add;
                xptr->Xleft = x;
                xptr->Gleft = g;
            }
        }
    }
    if (min_y == max_y)
        return (0);
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return (1);
}

typedef struct uvg_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
    uint16 u;
    uint16 v;
} UVG_INFO;

int xgen_xguv(sint16 *iptr)
{
    UVG_INFO *ptr1, *ptr2;
    XBUF_XGUV *xptr;
    int g, g_add;
    int x, x_add;
    int u, u_add;
    int v, v_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = (int)*(iptr++);
    ptr1 = (UVG_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0ffff;
            g = ptr1->shade << 8;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            u = ptr1->u << 8;
            u_add = ((ptr2->u << 8) - u) / ydif;
            v = ptr1->v << 8;
            v_add = ((ptr2->v << 8) - v) / ydif;
            xptr = ((XBUF_XGUV *)xbuffer) + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                g += g_add;
                u += u_add;
                v += v_add;
                xptr->Xright = x;
                xptr->Gright = g;
                xptr->Uright = u;
                xptr->Vright = v;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0001;
            g = ptr2->shade << 8;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            u = ptr2->u << 8;
            u_add = ((ptr1->u << 8) - u) / ydif;
            v = ptr2->v << 8;
            v_add = ((ptr1->v << 8) - v) / ydif;
            xptr = ((XBUF_XGUV *)xbuffer) + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                g += g_add;
                u += u_add;
                v += v_add;
                xptr->Xleft = x;
                xptr->Gleft = g;
                xptr->Uleft = u;
                xptr->Vleft = v;
            }
        }
    }
    if (min_y == max_y)
        return (0);
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return (1);
}

typedef struct xbuf_persp_fp {
    sint32 Xleft;
    sint32 Gleft;
    float UOZleft;
    float VOZleft;
    float OOZleft;
    sint32 Xright;
    sint32 Gright;
    float UOZright;
    float VOZright;
    float OOZright;
} XBUF_PERSP_FP;

typedef struct uvgp_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
    float ooz;
    float uoz;
    float voz;
} UVGP_INFO;

#pragma optimize("a", off)
int xgen_xguvpersp_fp(sint16 *iptr)
{
    UVGP_INFO *ptr1, *ptr2;
    XBUF_PERSP_FP *xptr;
    int g, g_add;
    int x, x_add;
    float scale;
    float uoz, uoz_add;
    float voz, voz_add;
    float ooz, ooz_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = (int)*(iptr++);
    ptr2 = (UVGP_INFO *)iptr;
    ptr1 = ptr2 + numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0ffff;
            g = ptr1->shade << 8;
            scale = 1.0f / ydif;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            uoz = ptr1->uoz;
            uoz_add = (ptr2->uoz - uoz) * scale;
            voz = ptr1->voz;
            voz_add = (ptr2->voz - voz) * scale;
            ooz = ptr1->ooz;
            ooz_add = (ptr2->ooz - ooz) * scale;
            xptr = ((XBUF_PERSP_FP *)xbuffer) + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                xptr->Xright = x;
                g += g_add;
                xptr->Gright = g;
                uoz += uoz_add;
                xptr->UOZright = uoz;
                voz += voz_add;
                xptr->VOZright = voz;
                ooz += ooz_add;
                xptr->OOZright = ooz;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0001;
            g = ptr2->shade << 8;
            scale = 1.0f / ydif;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            uoz = ptr2->uoz;
            uoz_add = (ptr1->uoz - uoz) * scale;
            voz = ptr2->voz;
            voz_add = (ptr1->voz - voz) * scale;
            ooz = ptr2->ooz;
            ooz_add = (ptr1->ooz - ooz) * scale;
            xptr = ((XBUF_PERSP_FP *)xbuffer) + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                xptr->Xleft = x;
                g += g_add;
                xptr->Gleft = g;
                uoz += uoz_add;
                xptr->UOZleft = uoz;
                voz += voz_add;
                xptr->VOZleft = voz;
                ooz += ooz_add;
                xptr->OOZleft = ooz;
            }
        }
    }
    if (min_y == max_y)
        return (0);
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return (1);
}
#pragma optimize("a", on)


#define SINGLE_MASK 0xffff8000

typedef struct {
    uint8 index[256];
} DEPTHQ_ENTRY;

extern "C" DEPTHQ_ENTRY DepthQTable[];
extern "C" sint32 phd_scrwidth;

void gtmap_persp32_fp(int ymin, int ymax, unsigned char *tptr)
{
    XBUF_PERSP_FP *dataptr;
    unsigned char *lineptr, *pixptr;
    float ooz, uoz, voz, scale;
    float oozadd32, uozadd32, vozadd32;
    int start_g, add_g, add_g2;
    int end_u, end_v;
    int start_u, start_v;
    int add_u, add_v;
    int start_x, end_x;
    int x;

    dataptr = ((XBUF_PERSP_FP *)xbuffer) + ymin;
    lineptr = (unsigned char *)phd_winptr + (ymin * phd_scrwidth);
    for (ymax -= ymin; ymax > 0; ymax--, dataptr++, lineptr += phd_scrwidth) {
        start_x = dataptr->Xleft >> 16;
        end_x = dataptr->Xright >> 16;
        x = end_x - start_x;
        if (x <= 0)
            continue;
        if (x > 32) {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            add_g2 = add_g << 1;
            scale = 32.0f / x;
            uoz = dataptr->UOZleft;
            uozadd32 = (dataptr->UOZright - uoz) * scale;
            voz = dataptr->VOZleft;
            vozadd32 = (dataptr->VOZright - voz) * scale;
            ooz = dataptr->OOZleft;
            oozadd32 = (dataptr->OOZright - ooz) * scale;
            scale = 256.0f / ooz;
            end_u = (int)(uoz * scale);
            end_v = (int)(voz * scale);
            pixptr = lineptr + start_x;
            ooz += oozadd32;
            scale = 256.0f / ooz;
            for (; x >= 32; x -= 32) {
                start_u = end_u;
                uoz += uozadd32;
                end_u = (int)(uoz * scale);
                add_u = (end_u - start_u) >> 5;
                start_v = end_v;
                voz += vozadd32;
                end_v = (int)(voz * scale);
                add_v = (end_v - start_v) >> 5;
                ooz += oozadd32;
                scale = 256.0f / ooz;
                if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                    *(pixptr) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 1) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 2) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 3) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 4) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 5) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 6) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 7) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 8) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 9) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 10) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 11) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 12) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 13) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 14) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 15) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_g += add_g;
                    start_u += add_u;
                    start_v += add_v;
                    *(pixptr + 16) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 17) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 18) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 19) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 20) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 21) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 22) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 23) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 24) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 25) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 26) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 27) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 28) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 29) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 30) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 31) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                } else {
                    add_u <<= 1;
                    add_v <<= 1;
                    *(pixptr + 0) = *(pixptr + 1) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 2) = *(pixptr + 3) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 4) = *(pixptr + 5) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 6) = *(pixptr + 7) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 8) = *(pixptr + 9) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 10) = *(pixptr + 11) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 12) = *(pixptr + 13) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 14) = *(pixptr + 15) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 16) = *(pixptr + 17) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 18) = *(pixptr + 19) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 20) = *(pixptr + 21) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 22) = *(pixptr + 23) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 24) = *(pixptr + 25) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 26) = *(pixptr + 27) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 28) = *(pixptr + 29) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 30) = *(pixptr + 31) = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
                pixptr += 32;
            }
            if (!x)
                continue;
            scale = 256.0f / dataptr->OOZright;
            start_u = end_u;
            end_u = (int)(dataptr->UOZright * scale);
            add_u = (end_u - start_u) / x;
            start_v = end_v;
            end_v = (int)(dataptr->VOZright * scale);
            add_v = (end_v - start_v) / x;
        } else {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            scale = 256.0f / dataptr->OOZleft;
            start_u = (int)(dataptr->UOZleft * scale);
            start_v = (int)(dataptr->VOZleft * scale);
            scale = 256.0f / dataptr->OOZright;
            end_u = (int)(dataptr->UOZright * scale);
            end_v = (int)(dataptr->VOZright * scale);
            add_u = (end_u - start_u) / x;
            add_v = (end_v - start_v) / x;
            pixptr = lineptr + start_x;
        }
        if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
            for (; x > 1; x -= 2, pixptr += 2) {
                pixptr[0] = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g;
                pixptr[1] = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g;
            }
        } else {
            add_u <<= 1;
            add_v <<= 1;
            add_g2 = add_g << 1;
            for (; x > 1; x -= 2, pixptr += 2) {
                pixptr[0] = pixptr[1] = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g2;
            }
        }
        if (x)
            pixptr[0] = DepthQTable[start_g >> 16].index[*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
    }
}


void wgtmap_persp32_fp(int ymin, int ymax, unsigned char *tptr)
{
    XBUF_PERSP_FP *dataptr;
    unsigned char *lineptr, *pixptr;
    float ooz, uoz, voz, scale;
    float oozadd32, uozadd32, vozadd32;
    int start_g, add_g, add_g2;
    int end_u, end_v;
    int start_u, start_v;
    int add_u, add_v;
    int start_x, end_x;
    int x;
    unsigned char texel;

    dataptr = ((XBUF_PERSP_FP *)xbuffer) + ymin;
    lineptr = (unsigned char *)phd_winptr + (ymin * phd_scrwidth);
    for (ymax -= ymin; ymax > 0; ymax--, dataptr++, lineptr += phd_scrwidth) {
        start_x = dataptr->Xleft >> 16;
        end_x = dataptr->Xright >> 16;
        x = end_x - start_x;
        if (x > 32) {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            add_g2 = add_g << 1;
            scale = 32.0f / x;
            uoz = dataptr->UOZleft;
            uozadd32 = (dataptr->UOZright - uoz) * scale;
            voz = dataptr->VOZleft;
            vozadd32 = (dataptr->VOZright - voz) * scale;
            ooz = dataptr->OOZleft;
            oozadd32 = (dataptr->OOZright - ooz) * scale;
            scale = 256.0f / ooz;
            end_u = (int)(uoz * scale);
            end_v = (int)(voz * scale);
            pixptr = lineptr + start_x;
            ooz += oozadd32;
            scale = 256.0f / ooz;
            for (; x >= 32; x -= 32) {
                start_u = end_u;
                uoz += uozadd32;
                end_u = (int)(uoz * scale);
                add_u = (end_u - start_u) >> 5;
                start_v = end_v;
                voz += vozadd32;
                end_v = (int)(voz * scale);
                add_v = (end_v - start_v) >> 5;
                ooz += oozadd32;
                scale = 256.0f / ooz;
                if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 1) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 2) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 3) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 4) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 5) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 6) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 7) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 8) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 9) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 10) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 11) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 12) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 13) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 14) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 15) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 16) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 17) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 18) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 19) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 20) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 21) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 22) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 23) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 24) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 25) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 26) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 27) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 28) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 29) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 30) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 31) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                } else {
                    add_u <<= 1;
                    add_v <<= 1;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 0) = *(pixptr + 1) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 2) = *(pixptr + 3) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 4) = *(pixptr + 5) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 6) = *(pixptr + 7) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 8) = *(pixptr + 9) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 10) = *(pixptr + 11) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 12) = *(pixptr + 13) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 14) = *(pixptr + 15) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 16) = *(pixptr + 17) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 18) = *(pixptr + 19) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 20) = *(pixptr + 21) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 22) = *(pixptr + 23) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 24) = *(pixptr + 25) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 26) = *(pixptr + 27) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 28) = *(pixptr + 29) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        *(pixptr + 30) = *(pixptr + 31) = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
                pixptr += 32;
            }
            if (!x)
                continue;
            scale = 256.0f / dataptr->OOZright;
            start_u = end_u;
            end_u = (int)(dataptr->UOZright * scale);
            add_u = (end_u - start_u) / x;
            start_v = end_v;
            end_v = (int)(dataptr->VOZright * scale);
            add_v = (end_v - start_v) / x;
            if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                for (; x > 1; x -= 2, pixptr += 2) {
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[0] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[1] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                }
            } else {
                add_u <<= 1;
                add_v <<= 1;
                add_g2 = add_g << 1;
                for (; x > 1; x -= 2, pixptr += 2) {
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[0] = pixptr[1] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
            }
            if (x) {
                texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                if (texel)
                    pixptr[0] = DepthQTable[start_g >> 16].index[texel];
            }
        } else {
            if (x <= 0)
                continue;
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            scale = 256.0f / dataptr->OOZleft;
            start_u = (int)(dataptr->UOZleft * scale);
            start_v = (int)(dataptr->VOZleft * scale);
            scale = 256.0f / dataptr->OOZright;
            end_u = (int)(dataptr->UOZright * scale);
            end_v = (int)(dataptr->VOZright * scale);
            add_u = (end_u - start_u) / x;
            add_v = (end_v - start_v) / x;
            pixptr = lineptr + start_x;
            if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                for (; x > 1; x -= 2, pixptr += 2) {
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[0] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[1] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                }
            } else {
                add_u <<= 1;
                add_v <<= 1;
                add_g2 = add_g << 1;
                for (; x > 1; x -= 2, pixptr += 2) {
                    texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                    if (texel)
                        pixptr[0] = pixptr[1] = DepthQTable[start_g >> 16].index[texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
            }
            if (x) {
                texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16));
                if (texel)
                    pixptr[0] = DepthQTable[start_g >> 16].index[texel];
            }
        }
    }
}


void draw_poly_gtmap_persp(sint16 *iptr)
{
    if (xgen_xguvpersp_fp(iptr + 1))
        gtmap_persp32_fp(xgen_ymin, xgen_ymax, (unsigned char *)texture_page_ptrs[*(iptr)]);
}

void draw_poly_wgtmap_persp(sint16 *iptr)
{
    if (xgen_xguvpersp_fp(iptr + 1))
        wgtmap_persp32_fp(xgen_ymin, xgen_ymax, (unsigned char *)texture_page_ptrs[*(iptr)]);
}

#ifdef TR2_EXE
char *phd_winptr;
DEPTHQ_ENTRY DepthQTable[32];
sint32 phd_scrwidth;
XBUF_PERSP xbuffer[1200];
char *texture_page_ptrs[32];
#endif
