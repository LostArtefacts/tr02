#include "../specific/include.h"
#include "../3dglodef.h"
#include "../specific/winmain.h"

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint16 clip, g, u, v;
} PHD_VBUF;

extern "C" PHD_VBUF PhdVBuf[];
extern "C" PHDTEXTURESTRUCT phdtextinfo[];

typedef struct {
    uint8 red, green, blue;
} RGB888;

extern "C" D3DTLVERTEX *HWR_VertexPtr;
extern "C" float RhwFactor;
extern "C" float FltResZBuf;
extern "C" float FltResZORhw;
extern "C" RGB888 game_palette[];
extern "C" sint32 *sort3dptr;
extern "C" sint16 *info3dptr;

bool HWR_VertexBufferFull();
void HWR_TexSource(D3DTEXTUREHANDLE tSource);
void HWR_EnableColorKey(bool tEnable);

extern "C" D3DTLVERTEX VBufferD3D[];

extern "C" void InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex, int sorttype);

extern "C" void InsertGT3_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *tex, uint16 *uv1, uint16 *uv2, uint16 *uv3, int sorttype);

extern "C" void InsertGT4_ZBuffered(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex);

extern "C" sint16 *InsertObjectGT4_ZBuffered(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *v1, *v2, *v3, *v4;

    while (number--) {
        v1 = &PhdVBuf[objptr[0]];
        v2 = &PhdVBuf[objptr[1]];
        v3 = &PhdVBuf[objptr[2]];
        v4 = &PhdVBuf[objptr[3]];
        if (phdtextinfo[objptr[4]].drawtype)
            InsertGT4_Sorted(v1, v2, v3, v4, &phdtextinfo[objptr[4]], sorttype);
        else
            InsertGT4_ZBuffered(v1, v2, v3, v4, &phdtextinfo[objptr[4]]);
        objptr += 5;
    }
    return objptr;
}

extern "C" void InsertGT3_ZBuffered(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *tex, uint16 *uv1, uint16 *uv2, uint16 *uv3);

extern "C" sint16 *InsertObjectGT3_ZBuffered(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *v1, *v2, *v3;
    PHDTEXTURESTRUCT *tex;

    while (number--) {
        v1 = &PhdVBuf[objptr[0]];
        v2 = &PhdVBuf[objptr[1]];
        v3 = &PhdVBuf[objptr[2]];
        tex = &phdtextinfo[objptr[3]];
        if (tex->drawtype)
            InsertGT3_Sorted(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3, sorttype);
        else
            InsertGT3_ZBuffered(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3);
        objptr += 4;
    }
    return objptr;
}

inline uint32 ShadeColor(int r, int g, int b)
{
    if (bBlueEffect) {
        r *= 128;
        r >>= 8;
        g *= 224;
        g >>= 8;
    }
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

extern "C" void InsertLine_ZBuffered(int x1, int y1, int x2, int y2, int z, uint8 color)
{
    uint8 *rgb;
    float rhw;

    if (z >= phd_zfar)
        return;
    if (z < phd_znear)
        z = phd_znear;
    rgb = (uint8 *)game_palette + color * 3;
    VBufferD3D[0].sx = (float)(phd_winxmin + x1);
    VBufferD3D[0].sy = (float)(phd_winymin + y1);
    VBufferD3D[1].sx = (float)(phd_winxmin + x2);
    VBufferD3D[1].sy = (float)(phd_winymin + y2);
    rhw = RhwFactor / (float)z;
    VBufferD3D[0].sz = VBufferD3D[1].sz = FltResZBuf - rhw * FltResZORhw;
    VBufferD3D[0].rhw = VBufferD3D[1].rhw = rhw;
    VBufferD3D[0].color = VBufferD3D[1].color = ShadeColor(rgb[0], rgb[1], rgb[2]);
    HWR_TexSource(0);
    HWR_EnableColorKey(false);
    G_Direct3DDevice->DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, VBufferD3D, 2, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

extern "C" sint16 *InsertObjectGT4_Sorted(sint16 *objptr, int number, int sorttype)
{
    while (number) {
        if (HWR_VertexBufferFull())
            return objptr + 5 * number;
        InsertGT4_Sorted(&PhdVBuf[objptr[0]], &PhdVBuf[objptr[1]], &PhdVBuf[objptr[2]], &PhdVBuf[objptr[3]], &phdtextinfo[objptr[4]], sorttype);
        number--;
        objptr += 5;
    }
    return objptr;
}

extern "C" sint16 *InsertObjectGT3_Sorted(sint16 *objptr, int number, int sorttype)
{
    PHDTEXTURESTRUCT *tex;

    if (HWR_VertexBufferFull())
        return objptr + 4 * number;
    while (number) {
        if (HWR_VertexBufferFull())
            return objptr + 4 * number;
        tex = &phdtextinfo[objptr[3]];
        InsertGT3_Sorted(&PhdVBuf[objptr[0]], &PhdVBuf[objptr[1]], &PhdVBuf[objptr[2]], tex, &tex->u1, &tex->u2, &tex->u3, sorttype);
        number--;
        objptr += 4;
    }
    return objptr;
}

extern "C" void InsertFlatRect_Sorted(int x1, int y1, int x2, int y2, int z, int color)
{
    uint8 *rgb;
    D3DTLVERTEX *v;

    if (x2 <= x1 || y2 <= y1)
        return;
    if (x1 < phd_winxmin)
        x1 = phd_winxmin;
    if (y1 < phd_winymin)
        y1 = phd_winymin;
    if (x2 > phd_winxmin + phd_winwidth)
        x2 = phd_winxmin + phd_winwidth;
    if (y2 > phd_winymin + phd_winheight)
        x2 = phd_winymin + phd_winheight;
    sort3dptr[0] = (sint32)info3dptr;
    rgb = (uint8 *)game_palette + color * 3;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 11;
    info3dptr[1] = 4;
    v = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 2) = v;
    info3dptr += 4;
    v[0].sx = v[3].sx = (float)x1;
    v[0].sy = v[1].sy = (float)y1;
    v[1].sx = v[2].sx = (float)x2;
    v[3].sy = v[2].sy = (float)y2;
    v[0].color = v[1].color = v[2].color = v[3].color = ShadeColor(rgb[0], rgb[1], rgb[2]);
    HWR_VertexPtr = v + 4;
    surfacenum++;
}

extern "C" void InsertTransQuad_Sorted(int x, int y, int width, int height, int z)
{
    D3DTLVERTEX *v;
    float rhw, sz;

    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 13;
    info3dptr[1] = 4;
    v = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 2) = v;
    info3dptr += 4;
    v[0].sx = v[3].sx = (float)x;
    v[0].sy = v[1].sy = (float)y;
    v[1].sx = v[2].sx = (float)(x + width);
    v[3].sy = v[2].sy = (float)(y + height);
    v[0].color = v[1].color = v[2].color = v[3].color = 0x80000000;
    if (SavedAppSettings.ZBuffer) {
        rhw = RhwFactor / (float)z;
        v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = rhw;
        sz = FltResZBuf - rhw * FltResZORhw;
        v[0].sz = v[1].sz = v[2].sz = v[3].sz = sz;
    }
    HWR_VertexPtr = v + 4;
    surfacenum++;
}
