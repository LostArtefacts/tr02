#include "../3dglodef.h"

extern "C" void __fastcall flatA(int min_y, int max_y, char color);
extern "C" void __fastcall transA(int min_y, int max_y, char color);
extern "C" void draw_poly_trans(sint16 *iptr);
extern "C" void __fastcall gourA(int min_y, int max_y, sint16 colbase);
extern "C" int xgen_xg(sint16 *iptr);
extern "C" void draw_poly_gouraud(sint16 *iptr);

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
