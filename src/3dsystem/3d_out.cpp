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
