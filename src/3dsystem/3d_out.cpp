#include "../3dglodef.h"

extern "C" void __fastcall flatA(int min_y, int max_y, char color);

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
