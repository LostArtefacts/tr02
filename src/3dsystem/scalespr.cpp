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

extern "C" sint32 *sort3dptr;
extern "C" sint16 *info3dptr;

extern "C" void insert_sprite(int z, int x1, int y1, int x2, int y2, int sprnum, sint16 shade)
{
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 8;
    info3dptr[1] = x1;
    info3dptr[2] = y1;
    info3dptr[3] = x2;
    info3dptr[4] = y2;
    info3dptr[5] = sprnum;
    info3dptr[6] = shade;
    info3dptr += 7;
    surfacenum++;
}

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

extern "C" void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags)
{
    PHDSPRITESTRUCT *sprite;
    int x1, y1, x2, y2;

    sprite = &PhdSpriteInfo[sprnum];
    x1 = sx + ((scaleH * (sprite->x1 >> 3)) >> 16);
    x2 = sx + ((scaleH * (sprite->x2 >> 3)) >> 16);
    y1 = sy + ((scaleV * (sprite->y1 >> 3)) >> 16);
    y2 = sy + ((scaleV * (sprite->y2 >> 3)) >> 16);
    if (x2 >= 0 && y2 >= 0 && x1 < phd_winwidth && y1 < phd_winheight)
        InsertSprite(z << 3, x1, y1, x2, y2, sprnum, shade);
}

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint8 clip, clip_hi;
    sint16 g, u, v;
} PHD_VBUF;

extern "C" PHD_VBUF PhdVBuf[];
extern "C" sint32 PhdPersp;

extern "C" sint16 *ins_room_sprite(sint16 *objptr, int num)
{
    PHD_VBUF *vn;
    sint32 x1, y1, x2, y2, zv;
    PHDSPRITESTRUCT *sptr;

    for (; num > 0; num--, objptr += 2) {
        vn = &PhdVBuf[*objptr];
        if (vn->clip < 0)
            continue;
        zv = (sint32)vn->zv / PhdPersp;
        sptr = &PhdSpriteInfo[objptr[1]];
        x1 = (sint32)((vn->xv + (sptr->x1 << W2V_SHIFT)) / zv + phd_centerx);
        y1 = (sint32)((vn->yv + (sptr->y1 << W2V_SHIFT)) / zv + phd_centery);
        x2 = (sint32)((vn->xv + (sptr->x2 << W2V_SHIFT)) / zv + phd_centerx);
        y2 = (sint32)((vn->yv + (sptr->y2 << W2V_SHIFT)) / zv + phd_centery);
        if (x2 < phd_left || y2 < phd_top || x1 >= phd_right || y1 >= phd_bottom)
            continue;
        InsertSprite((int)vn->zv, x1, y1, x2, y2, objptr[1], vn->g);
    }
    return objptr;
}
