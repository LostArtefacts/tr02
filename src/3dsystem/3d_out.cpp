#include "../3dglodef.h"

typedef struct xbuf_x {
    sint32 Xleft;
    sint32 Xright;
} XBUF_X;

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
