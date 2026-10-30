#include "../3dglodef.h"

typedef struct {
    uint16 tpage;
    uint16 offset;
    uint16 width;
    uint16 height;
    sint16 x1;
    sint16 y1;
    sint16 x2;
    sint16 y2;
} PHDSPRITESTRUCT;

extern "C" PHDSPRITESTRUCT PhdSpriteInfo[];
extern "C" void (*InsertSprite)(int z, int x1, int y1, int x2, int y2, int sprnum, int shade);

extern "C" void S_DrawPickup(int sx, int sy, int scale, sint16 sprnum, sint16 shade)
{
    PHDSPRITESTRUCT *sprite;
    int x1, y1, x2, y2;

    sprite = &PhdSpriteInfo[sprnum];
    x1 = sx + ((scale * sprite->x1) >> 16);
    x2 = sx + ((scale * sprite->x2) >> 16);
    y1 = sy + ((scale * sprite->y1) >> 16);
    y2 = sy + ((scale * sprite->y2) >> 16);
    if (x2 >= 0 && y2 >= 0 && x1 < phd_winwidth && y1 < phd_winheight)
        InsertSprite(200, x1, y1, x2, y2, sprnum, shade);
}

extern "C" void S_DrawScreenSprite2d(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags)
{
    PHDSPRITESTRUCT *sprite;
    int x1, y1, x2, y2;

    sprite = &PhdSpriteInfo[sprnum];
    x1 = sx + ((scaleH * sprite->x1) >> 16);
    y1 = sy + ((scaleV * sprite->y1) >> 16);
    x2 = sx + ((scaleH * sprite->x2) >> 16);
    y2 = sy + ((scaleV * sprite->y2) >> 16);
    if (x2 >= 0 && y2 >= 0 && x1 < phd_winwidth && y1 < phd_winheight)
        InsertSprite(phd_znear + (z << 3), x1, y1, x2, y2, sprnum, shade);
}
