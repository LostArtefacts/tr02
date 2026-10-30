#include "../3dglodef.h"
#include "../game.h"

extern "C" {
extern sint32 phd_scrwidth;
void draw_poly_line(sint16 *iptr);
}

void draw_poly_line(sint16 *iptr)
{
    sint32 i, frak, fraksum, xdif, ydif, xadd, yadd;
    sint32 x1, y1, x2, y2;
    char lcolor;
    char *drawptr;

    x1 = (sint32)(*(iptr + 0));
    y1 = (sint32)(*(iptr + 1));
    x2 = (sint32)(*(iptr + 2));
    y2 = (sint32)(*(iptr + 3));
    lcolor = (char)(*(iptr + 4));
    if (x2 < x1) {
        i = x2;
        x2 = x1;
        x1 = i;
        i = y2;
        y2 = y1;
        y1 = i;
    }
    if (x2 < 0 || x1 > phd_winxmax)
        return;
    if (x1 < 0) {
        y1 -= ((y2 - y1) * x1) / (x2 - x1);
        x1 = 0;
    }
    if (x2 > phd_winxmax) {
        y2 = y1 + ((phd_winxmax - x1) * (y2 - y1)) / (x2 - x1);
        x2 = phd_winxmax;
    }
    if (y2 < y1) {
        i = x2;
        x2 = x1;
        x1 = i;
        i = y2;
        y2 = y1;
        y1 = i;
    }
    if (y2 < 0 || y1 > phd_winymax)
        return;
    if (y1 < 0) {
        x1 -= ((x2 - x1) * y1) / (y2 - y1);
        y1 = 0;
    }
    if (y2 > phd_winymax) {
        x2 = x1 + ((x2 - x1) * (phd_winymax - y1)) / (y2 - y1);
        y2 = phd_winymax;
    }
    drawptr = phd_winptr + (phd_scrwidth * y1) + x1;
    xdif = x2 - x1;
    ydif = y2 - y1;
    if (xdif || ydif) {
        if (xdif >= 0)
            xadd = 1;
        else {
            xadd = -1;
            xdif = -xdif;
        }
        if (ydif >= 0)
            yadd = phd_scrwidth;
        else {
            yadd = -phd_scrwidth;
            ydif = -ydif;
        }
        fraksum = 0;
        if (xdif++ < ydif++) {
            frak = ((xdif << 16) / ydif);
            for (i = 0; i < ydif; i++) {
                *(drawptr) = lcolor;
                drawptr += yadd;
                fraksum += frak;
                if (fraksum >= 65536) {
                    drawptr += xadd;
                    fraksum -= 65536;
                }
            }
        } else {
            frak = ((ydif << 16) / xdif);
            for (i = 0; i < xdif; i++) {
                *(drawptr) = lcolor;
                drawptr += xadd;
                fraksum += frak;
                if (fraksum >= 65536) {
                    drawptr += yadd;
                    fraksum -= 65536;
                }
            }
        }
    } else
        *drawptr = lcolor;
}
