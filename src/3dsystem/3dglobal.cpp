#include "../3dglodef.h"

#ifdef TR2_EXE
extern "C" {
void draw_poly_gtmap(sint16 *iptr);
void draw_poly_wgtmap(sint16 *iptr);
void draw_poly_gtmap_persp(sint16 *iptr);
void draw_poly_wgtmap_persp(sint16 *iptr);
void draw_poly_line(sint16 *iptr);
void draw_poly_gouraud(sint16 *iptr);
void draw_poly_trans(sint16 *iptr);
void draw_scaled_spriteC(sint16 *inptr);

int perspective_distance = 50331648;
void (*poly_draw_routines[9])(sint16 *) = {
    draw_poly_gtmap,
    draw_poly_wgtmap,
    draw_poly_gtmap_persp,
    draw_poly_wgtmap_persp,
    draw_poly_line,
    draw_poly_flat,
    draw_poly_gouraud,
    draw_poly_trans,
    draw_scaled_spriteC,
};
}
#endif
