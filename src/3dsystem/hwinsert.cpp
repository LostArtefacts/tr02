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

typedef struct {
    float x, y, rhw, u, v, g;
} VERTEX_INFO;

extern "C" VERTEX_INFO VBuffer[];

inline uint32 ShadeU(uint32 r, uint32 g, uint32 b)
{
    if (bBlueEffect) {
        r *= 128;
        r >>= 8;
        g *= 224;
        g >>= 8;
    }
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

typedef struct {
    float xv, yv, zv, ooz, xs, ys, u, v, g;
} POINT_INFO;

extern "C" int ZedClipper(int number, POINT_INFO *input, VERTEX_INFO *output);
extern "C" int XYGUVClipper(int number, VERTEX_INFO *input);
extern "C" int visible_zclip(PHD_VBUF *vn1, PHD_VBUF *vn2, PHD_VBUF *vn3);
extern "C" void DrawClippedPoly_Textured(int npoints);
extern "C" D3DTEXTUREHANDLE HWR_PageHandles[];

extern "C" void InsertGT3_ZBuffered(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *tex, uint16 *uv1, uint16 *uv2, uint16 *uv3)
{
    POINT_INFO point[3];
    int npoints;
    uint32 shade;

    if ((sint8)(v1->clip & v2->clip & v3->clip))
        return;
    if ((sint8)(v1->clip | v2->clip | v3->clip) < 0) {
        if (!visible_zclip(v1, v2, v3))
            return;
        point[0].xv = v1->xv;
        point[0].yv = v1->yv;
        point[0].zv = v1->zv;
        point[0].ooz = v1->ooz;
        point[0].xs = v1->xs;
        point[0].ys = v1->ys;
        point[0].g = (float)v1->g;
        point[0].u = (float)uv1[0];
        point[0].v = (float)uv1[1];
        point[1].xv = v2->xv;
        point[1].yv = v2->yv;
        point[1].zv = v2->zv;
        point[1].ooz = v2->ooz;
        point[1].xs = v2->xs;
        point[1].ys = v2->ys;
        point[1].g = (float)v2->g;
        point[1].u = (float)uv2[0];
        point[1].v = (float)uv2[1];
        point[2].xv = v3->xv;
        point[2].yv = v3->yv;
        point[2].zv = v3->zv;
        point[2].ooz = v3->ooz;
        point[2].xs = v3->xs;
        point[2].ys = v3->ys;
        point[2].g = (float)v3->g;
        point[2].u = (float)uv3[0];
        point[2].v = (float)uv3[1];
        npoints = ZedClipper(3, point, VBuffer);
        if (!npoints)
            return;
    } else {
        if ((sint32)((v3->xs - v2->xs) * (v1->ys - v2->ys) - (v3->ys - v2->ys) * (v1->xs - v2->xs)) < 0)
            return;
        if ((sint8)(v1->clip | v2->clip | v3->clip)) {
            VBuffer[0].x = v1->xs;
            VBuffer[0].y = v1->ys;
            VBuffer[0].rhw = v1->ooz;
            VBuffer[0].g = (float)v1->g;
            VBuffer[0].u = uv1[0] * v1->ooz;
            VBuffer[0].v = uv1[1] * v1->ooz;
            VBuffer[1].x = v2->xs;
            VBuffer[1].y = v2->ys;
            VBuffer[1].rhw = v2->ooz;
            VBuffer[1].g = (float)v2->g;
            VBuffer[1].u = uv2[0] * v2->ooz;
            VBuffer[1].v = uv2[1] * v2->ooz;
            VBuffer[2].x = v3->xs;
            VBuffer[2].y = v3->ys;
            VBuffer[2].rhw = v3->ooz;
            VBuffer[2].g = (float)v3->g;
            VBuffer[2].u = uv3[0] * v3->ooz;
            VBuffer[2].v = uv3[1] * v3->ooz;
            npoints = 3;
        } else {
            VBufferD3D[0].sx = v1->xs;
            VBufferD3D[0].sy = v1->ys;
            VBufferD3D[0].sz = FltResZBuf - FltResZORhw * v1->ooz;
            VBufferD3D[0].rhw = v1->ooz;
            shade = (0x1fff - v1->g) >> 4;
            if (shade > 255)
                shade = 255;
            if (bBlueEffect)
                VBufferD3D[0].color = 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade;
            else
                VBufferD3D[0].color = 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[0].tu = uv1[0] / 65536.0f;
            VBufferD3D[0].tv = uv1[1] / 65536.0f;
            VBufferD3D[1].sx = v2->xs;
            VBufferD3D[1].sy = v2->ys;
            VBufferD3D[1].sz = FltResZBuf - FltResZORhw * v2->ooz;
            VBufferD3D[1].rhw = v2->ooz;
            shade = (0x1fff - v2->g) >> 4;
            if (shade > 255)
                shade = 255;
            if (bBlueEffect)
                VBufferD3D[1].color = 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade;
            else
                VBufferD3D[1].color = 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[1].tu = uv2[0] / 65536.0f;
            VBufferD3D[1].tv = uv2[1] / 65536.0f;
            VBufferD3D[2].sx = v3->xs;
            VBufferD3D[2].sy = v3->ys;
            VBufferD3D[2].sz = FltResZBuf - FltResZORhw * v3->ooz;
            VBufferD3D[2].rhw = v3->ooz;
            shade = (0x1fff - v3->g) >> 4;
            if (shade > 255)
                shade = 255;
            if (bBlueEffect)
                VBufferD3D[2].color = 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade;
            else
                VBufferD3D[2].color = 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[2].tu = uv3[0] / 65536.0f;
            VBufferD3D[2].tv = uv3[1] / 65536.0f;
            HWR_TexSource(HWR_PageHandles[tex->tpage]);
            HWR_EnableColorKey(tex->drawtype != 0);
            G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, VBufferD3D, 3, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            return;
        }
    }
    npoints = XYGUVClipper(npoints, VBuffer);
    if (!npoints)
        return;
    HWR_TexSource(HWR_PageHandles[tex->tpage]);
    HWR_EnableColorKey(tex->drawtype != 0);
    DrawClippedPoly_Textured(npoints);
}

extern "C" void DrawClippedPoly_Textured(int vtx_count)
{
    int i;
    uint32 shade;
    float k, tu, tv;
    VERTEX_INFO *vbuf;
    D3DTLVERTEX *vtx;

    vbuf = VBuffer;
    vtx = VBufferD3D;
    for (i = vtx_count; i; i--) {
        vtx->sx = vbuf->x;
        vtx->sy = vbuf->y;
        vtx->sz = FltResZBuf - FltResZORhw * vbuf->rhw;
        vtx->rhw = vbuf->rhw;
        shade = (0x1fff - (int)vbuf->g) >> 4;
        if (shade > 255)
            shade = 255;
        vtx->color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
        k = 1.0f / 65536.0f / vbuf->rhw;
        tu = vbuf->u * k;
        tv = k * vbuf->v;
        if (tu < 0.0f)
            tu = 0.0f;
        else if (tu > 1.0f)
            tu = 1.0f;
        if (tv < 0.0f)
            tv = 0.0f;
        else if (tv > 1.0f)
            tv = 1.0f;
        vtx->tv = tv;
        vtx->tu = tu;
        vbuf++;
        vtx++;
    }
    G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VBufferD3D, vtx_count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

extern "C" void InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex, int sorttype);

extern "C" void InsertGT3_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *tex, uint16 *uv1, uint16 *uv2, uint16 *uv3, int sorttype);

extern "C" void InsertGT4_ZBuffered(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex)
{
    uint32 shade;

    if ((sint8)(v1->clip & v2->clip & v3->clip & v4->clip))
        return;
    if ((sint8)(v1->clip | v2->clip | v3->clip | v4->clip) < 0) {
        if (visible_zclip(v1, v2, v3)) {
            InsertGT3_ZBuffered(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3);
            InsertGT3_ZBuffered(v1, v3, v4, tex, &tex->u1, &tex->u3, &tex->u4);
        }
    } else if ((sint32)((v3->xs - v2->xs) * (v1->ys - v2->ys) - (v3->ys - v2->ys) * (v1->xs - v2->xs)) >= 0) {
        if ((sint8)(v1->clip | v2->clip | v3->clip | v4->clip)) {
            InsertGT3_ZBuffered(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3);
            InsertGT3_ZBuffered(v1, v3, v4, tex, &tex->u1, &tex->u3, &tex->u4);
        } else {
            VBufferD3D[0].sx = v1->xs;
            VBufferD3D[0].sy = v1->ys;
            VBufferD3D[0].sz = FltResZBuf - FltResZORhw * v1->ooz;
            VBufferD3D[0].rhw = v1->ooz;
            shade = (0x1fff - v1->g) >> 4;
            if (shade > 255)
                shade = 255;
                        VBufferD3D[0].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[0].tu = tex->u1 / 65536.0f;
            VBufferD3D[0].tv = tex->v1 / 65536.0f;
            VBufferD3D[1].sx = v2->xs;
            VBufferD3D[1].sy = v2->ys;
            VBufferD3D[1].sz = FltResZBuf - FltResZORhw * v2->ooz;
            VBufferD3D[1].rhw = v2->ooz;
            shade = (0x1fff - v2->g) >> 4;
            if (shade > 255)
                shade = 255;
                        VBufferD3D[1].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[1].tu = tex->u2 / 65536.0f;
            VBufferD3D[1].tv = tex->v2 / 65536.0f;
            VBufferD3D[2].sx = v3->xs;
            VBufferD3D[2].sy = v3->ys;
            VBufferD3D[2].sz = FltResZBuf - FltResZORhw * v3->ooz;
            VBufferD3D[2].rhw = v3->ooz;
            shade = (0x1fff - v3->g) >> 4;
            if (shade > 255)
                shade = 255;
                        VBufferD3D[2].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[2].tu = tex->u3 / 65536.0f;
            VBufferD3D[2].tv = tex->v3 / 65536.0f;
            VBufferD3D[3].sx = v4->xs;
            VBufferD3D[3].sy = v4->ys;
            VBufferD3D[3].sz = FltResZBuf - FltResZORhw * v4->ooz;
            VBufferD3D[3].rhw = v4->ooz;
            shade = (0x1fff - v4->g) >> 4;
            if (shade > 255)
                shade = 255;
                        VBufferD3D[3].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            VBufferD3D[3].tu = tex->u4 / 65536.0f;
            VBufferD3D[3].tv = tex->v4 / 65536.0f;
            HWR_TexSource(HWR_PageHandles[tex->tpage]);
            HWR_EnableColorKey(tex->drawtype != 0);
            G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VBufferD3D, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        }
    }
}

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

extern "C" PALETTEENTRY GamePalette16[];
extern "C" int XYGClipper(int number, VERTEX_INFO *input);

extern "C" void DrawPoly_Gouraud(int vtx_count, int red, int green, int blue);

extern "C" sint16 *InsertObjectG4_ZBuffered(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *vn1, *vn2, *vn3, *vn4;
    POINT_INFO point[4];
    PALETTEENTRY *pal;
    int npoints;

    HWR_TexSource(0);
    HWR_EnableColorKey(false);
    for (; number; number--, objptr += 5) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        vn4 = &PhdVBuf[objptr[3]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip & vn4->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip | vn4->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            point[0].xv = vn1->xv;
            point[0].yv = vn1->yv;
            point[0].zv = vn1->zv;
            point[0].ooz = vn1->ooz;
            point[0].xs = vn1->xs;
            point[0].ys = vn1->ys;
            point[0].g = (float)vn1->g;
            point[1].xv = vn2->xv;
            point[1].yv = vn2->yv;
            point[1].zv = vn2->zv;
            point[1].ooz = vn2->ooz;
            point[1].xs = vn2->xs;
            point[1].ys = vn2->ys;
            point[1].g = (float)vn2->g;
            point[2].xv = vn3->xv;
            point[2].yv = vn3->yv;
            point[2].zv = vn3->zv;
            point[2].ooz = vn3->ooz;
            point[2].xs = vn3->xs;
            point[2].ys = vn3->ys;
            point[2].g = (float)vn3->g;
            point[3].xv = vn4->xv;
            point[3].yv = vn4->yv;
            point[3].zv = vn4->zv;
            point[3].ooz = vn4->ooz;
            point[3].xs = vn4->xs;
            point[3].ys = vn4->ys;
            point[3].g = (float)vn4->g;
            npoints = ZedClipper(4, point, VBuffer);
            if (!npoints)
                continue;
            npoints = XYGClipper(npoints, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            VBuffer[0].x = vn1->xs;
            VBuffer[0].y = vn1->ys;
            VBuffer[0].rhw = vn1->ooz;
            VBuffer[0].g = (float)vn1->g;
            VBuffer[1].x = vn2->xs;
            VBuffer[1].y = vn2->ys;
            VBuffer[1].rhw = vn2->ooz;
            VBuffer[1].g = (float)vn2->g;
            VBuffer[2].x = vn3->xs;
            VBuffer[2].y = vn3->ys;
            VBuffer[2].rhw = vn3->ooz;
            VBuffer[2].g = (float)vn3->g;
            VBuffer[3].x = vn4->xs;
            VBuffer[3].y = vn4->ys;
            VBuffer[3].rhw = vn4->ooz;
            VBuffer[3].g = (float)vn4->g;
            npoints = 4;
            if (vn1->clip | vn2->clip | vn3->clip | vn4->clip)
                npoints = XYGClipper(4, VBuffer);
        }
        if (npoints) {
            pal = &GamePalette16[(uint16)objptr[4] >> 8];
            DrawPoly_Gouraud(npoints, pal->peRed, pal->peGreen, pal->peBlue);
        }
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

extern "C" void DrawPoly_Gouraud(int vtx_count, int red, int green, int blue)
{
    int i, r, g, b;
    uint32 shade;
    VERTEX_INFO *vbuf;
    D3DTLVERTEX *vtx;

    vbuf = VBuffer;
    vtx = VBufferD3D;
    for (i = vtx_count; i; i--) {
        vtx->sx = vbuf->x;
        vtx->sy = vbuf->y;
        vtx->sz = FltResZBuf - FltResZORhw * vbuf->rhw;
        vtx->rhw = vbuf->rhw;
        shade = 0x1fff - (int)vbuf->g;
        r = (red * shade) >> 12;
        if (r > 255)
            r = 255;
        g = (green * shade) >> 12;
        if (g > 255)
            g = 255;
        b = (blue * shade) >> 12;
        if (b > 255)
            b = 255;
        vtx->color = ShadeColor(r, g, b);
        vbuf++;
        vtx++;
    }
    G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VBufferD3D, vtx_count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

extern "C" sint16 *InsertObjectG3_ZBuffered(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *vn1, *vn2, *vn3;
    POINT_INFO point[3];
    PALETTEENTRY *pal;
    int npoints;

    HWR_TexSource(0);
    HWR_EnableColorKey(false);
    for (; number; number--, objptr += 4) {
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            point[0].xv = vn1->xv;
            point[0].yv = vn1->yv;
            point[0].zv = vn1->zv;
            point[0].ooz = vn1->ooz;
            point[0].xs = vn1->xs;
            point[0].ys = vn1->ys;
            point[0].g = (float)vn1->g;
            point[1].xv = vn2->xv;
            point[1].yv = vn2->yv;
            point[1].zv = vn2->zv;
            point[1].ooz = vn2->ooz;
            point[1].xs = vn2->xs;
            point[1].ys = vn2->ys;
            point[1].g = (float)vn2->g;
            point[2].xv = vn3->xv;
            point[2].yv = vn3->yv;
            point[2].zv = vn3->zv;
            point[2].ooz = vn3->ooz;
            point[2].xs = vn3->xs;
            point[2].ys = vn3->ys;
            point[2].g = (float)vn3->g;
            npoints = ZedClipper(3, point, VBuffer);
            if (!npoints)
                continue;
            npoints = XYGClipper(npoints, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            VBuffer[0].x = vn1->xs;
            VBuffer[0].y = vn1->ys;
            VBuffer[0].rhw = vn1->ooz;
            VBuffer[0].g = (float)vn1->g;
            VBuffer[1].x = vn2->xs;
            VBuffer[1].y = vn2->ys;
            VBuffer[1].rhw = vn2->ooz;
            VBuffer[1].g = (float)vn2->g;
            VBuffer[2].x = vn3->xs;
            VBuffer[2].y = vn3->ys;
            VBuffer[2].rhw = vn3->ooz;
            VBuffer[2].g = (float)vn3->g;
            npoints = 3;
            if (vn1->clip | vn2->clip | vn3->clip)
                npoints = XYGClipper(3, VBuffer);
        }
        if (npoints) {
            pal = &GamePalette16[(uint16)objptr[3] >> 8];
            DrawPoly_Gouraud(npoints, pal->peRed, pal->peGreen, pal->peBlue);
        }
    }
    return objptr;
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

extern "C" void InsertFlatRect_ZBuffered(int x1, int y1, int x2, int y2, int z, int color)
{
    uint8 *rgb;
    float rhw;

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
    if (z < phd_znear)
        z = phd_znear;
    else if (z >= phd_zfar)
        z = phd_zfar;
    rhw = RhwFactor / (float)z;
    rgb = (uint8 *)game_palette + color * 3;
    VBufferD3D[0].sx = VBufferD3D[2].sx = (float)x1;
    VBufferD3D[0].sy = VBufferD3D[1].sy = (float)y1;
    VBufferD3D[1].sx = VBufferD3D[3].sx = (float)x2;
    VBufferD3D[2].sy = VBufferD3D[3].sy = (float)y2;
    VBufferD3D[0].sz = VBufferD3D[1].sz = VBufferD3D[2].sz = VBufferD3D[3].sz = FltResZBuf - rhw * FltResZORhw;
    VBufferD3D[0].rhw = VBufferD3D[1].rhw = VBufferD3D[2].rhw = VBufferD3D[3].rhw = rhw;
    VBufferD3D[0].color = VBufferD3D[1].color = VBufferD3D[2].color = VBufferD3D[3].color = ShadeColor(rgb[0], rgb[1], rgb[2]);
    HWR_TexSource(0);
    HWR_EnableColorKey(false);
    G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, VBufferD3D, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

extern "C" void InsertClippedPoly_Textured(int npoints, float zdepth, int drawtype, int tpage);

extern "C" void InsertGT3_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *tex, uint16 *uv1, uint16 *uv2, uint16 *uv3, int sorttype)
{
    POINT_INFO point[3];
    int npoints;
    uint32 shade;
    float zdepth;
    D3DTLVERTEX *vtx;

    if ((sint8)(v1->clip & v2->clip & v3->clip))
        return;
    if ((sint8)(v1->clip | v2->clip | v3->clip) < 0) {
        if (!visible_zclip(v1, v2, v3))
            return;
        point[0].xv = v1->xv;
        point[0].yv = v1->yv;
        point[0].zv = v1->zv;
        point[0].ooz = v1->ooz;
        point[0].xs = v1->xs;
        point[0].ys = v1->ys;
        point[0].g = (float)v1->g;
        point[0].u = (float)uv1[0];
        point[0].v = (float)uv1[1];
        point[1].xv = v2->xv;
        point[1].yv = v2->yv;
        point[1].zv = v2->zv;
        point[1].ooz = v2->ooz;
        point[1].xs = v2->xs;
        point[1].ys = v2->ys;
        point[1].g = (float)v2->g;
        point[1].u = (float)uv2[0];
        point[1].v = (float)uv2[1];
        point[2].xv = v3->xv;
        point[2].yv = v3->yv;
        point[2].zv = v3->zv;
        point[2].ooz = v3->ooz;
        point[2].xs = v3->xs;
        point[2].ys = v3->ys;
        point[2].g = (float)v3->g;
        point[2].u = (float)uv3[0];
        point[2].v = (float)uv3[1];
        npoints = ZedClipper(3, point, VBuffer);
        if (npoints)
            goto clip;
        return;
    } else {
        if ((sint32)((v3->xs - v2->xs) * (v1->ys - v2->ys) - (v3->ys - v2->ys) * (v1->xs - v2->xs)) < 0)
            return;
        if ((sint8)(v1->clip | v2->clip | v3->clip)) {
            VBuffer[0].x = v1->xs;
            VBuffer[0].y = v1->ys;
            VBuffer[0].rhw = v1->ooz;
            VBuffer[0].g = (float)v1->g;
            VBuffer[0].u = uv1[0] * v1->ooz;
            VBuffer[0].v = uv1[1] * v1->ooz;
            VBuffer[1].x = v2->xs;
            VBuffer[1].y = v2->ys;
            VBuffer[1].rhw = v2->ooz;
            VBuffer[1].g = (float)v2->g;
            VBuffer[1].u = uv2[0] * v2->ooz;
            VBuffer[1].v = uv2[1] * v2->ooz;
            VBuffer[2].x = v3->xs;
            VBuffer[2].y = v3->ys;
            VBuffer[2].rhw = v3->ooz;
            VBuffer[2].g = (float)v3->g;
            VBuffer[2].u = uv3[0] * v3->ooz;
            VBuffer[2].v = uv3[1] * v3->ooz;
            npoints = 3;
        } else {
            switch (sorttype) {
            case MID_SORT:
                zdepth = (v1->zv + v2->zv + v3->zv) * 0.333333333333333333f;
                break;
            case FAR_SORT:
                zdepth = v1->zv;
                if (v2->zv > zdepth)
                    zdepth = v2->zv;
                if (v3->zv > zdepth)
                    zdepth = v3->zv;
                break;
            default:
                zdepth = 1000000000.0f;
                break;
            }
            sort3dptr[0] = (sint32)info3dptr;
            sort3dptr[1] = (sint32)zdepth;
            sort3dptr += 2;
            info3dptr[0] = tex->drawtype + 9;
            info3dptr[1] = tex->tpage;
            info3dptr[2] = 3;
            vtx = HWR_VertexPtr;
            *(D3DTLVERTEX **)(info3dptr + 3) = vtx;
            info3dptr += 5;
            vtx[0].sx = v1->xs;
            vtx[0].sy = v1->ys;
            vtx[0].rhw = v1->ooz;
            shade = (0x1fff - v1->g) >> 4;
            if (shade > 255)
                shade = 255;
            vtx[0].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            vtx[0].tu = uv1[0] / 65536.0f;
            vtx[0].tv = uv1[1] / 65536.0f;
            vtx++;
            vtx[0].sx = v2->xs;
            vtx[0].sy = v2->ys;
            vtx[0].rhw = v2->ooz;
            shade = (0x1fff - v2->g) >> 4;
            if (shade > 255)
                shade = 255;
            vtx[0].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            vtx[0].tu = uv2[0] / 65536.0f;
            vtx[0].tv = uv2[1] / 65536.0f;
            vtx++;
            vtx[0].sx = v3->xs;
            vtx[0].sy = v3->ys;
            vtx[0].rhw = v3->ooz;
            shade = (0x1fff - v3->g) >> 4;
            if (shade > 255)
                shade = 255;
            vtx[0].color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
            vtx[0].tu = uv3[0] / 65536.0f;
            vtx[0].tv = uv3[1] / 65536.0f;
            if (SavedAppSettings.ZBuffer) {
                vtx[-2].sz = FltResZBuf - FltResZORhw * v1->ooz;
                vtx[-1].sz = FltResZBuf - FltResZORhw * v2->ooz;
                vtx[0].sz = FltResZBuf - FltResZORhw * v3->ooz;
            }
            HWR_VertexPtr = vtx + 1;
            surfacenum++;
            return;
        }
    }
clip:
    npoints = XYGUVClipper(npoints, VBuffer);
    if (!npoints)
        return;
    switch (sorttype) {
    case MID_SORT:
        zdepth = (v1->zv + v2->zv + v3->zv) * 0.333333333333333333f;
        break;
    case FAR_SORT:
        zdepth = v1->zv;
        if (v2->zv > zdepth)
            zdepth = v2->zv;
        if (v3->zv > zdepth)
            zdepth = v3->zv;
        break;
    default:
        zdepth = 1000000000.0f;
        break;
    }
    InsertClippedPoly_Textured(npoints, zdepth, tex->drawtype + 9, tex->tpage);
}

extern "C" void InsertClippedPoly_Textured(int vtx_count, float z, int poly_type, int tex_page)
{
    uint32 shade;
    bool zbuf;
    float k, tu, tv;
    D3DTLVERTEX *vtx;
    int i;
    VERTEX_INFO *vbuf;

    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = (sint32)z;
    sort3dptr += 2;
    info3dptr[0] = poly_type;
    info3dptr[1] = tex_page;
    info3dptr[2] = vtx_count;
    vtx = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 3) = vtx;
    info3dptr += 5;
    zbuf = SavedAppSettings.ZBuffer;
    vbuf = VBuffer;
    for (i = vtx_count; i; i--) {
        vtx->sx = vbuf->x;
        vtx->sy = vbuf->y;
        if (zbuf)
            vtx->sz = FltResZBuf - FltResZORhw * vbuf->rhw;
        vtx->rhw = vbuf->rhw;
        shade = (0x1fff - (int)vbuf->g) >> 4;
        if (shade > 255)
            shade = 255;
        vtx->color = ShadeU(shade, shade, shade);
        k = 1.0f / 65536.0f / vbuf->rhw;
        tu = vbuf->u * k;
        tv = k * vbuf->v;
        if (tu < 0.0f)
            tu = 0.0f;
        else if (tu > 1.0f)
            tu = 1.0f;
        if (tv < 0.0f)
            tv = 0.0f;
        else if (tv > 1.0f)
            tv = 1.0f;
        vtx->tv = tv;
        vtx->tu = tu;
        vbuf++;
        vtx++;
    }
    HWR_VertexPtr = vtx;
    surfacenum++;
}

extern "C" void InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex, int sorttype)
{
    float zdepth;
    uint32 shade;
    D3DTLVERTEX *vtx;

    if ((sint8)(v1->clip & v2->clip & v3->clip & v4->clip))
        return;
    if ((sint8)(v1->clip | v2->clip | v3->clip | v4->clip) < 0) {
        if (!visible_zclip(v1, v2, v3))
    return;
        InsertGT3_Sorted(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3, sorttype);
        InsertGT3_Sorted(v1, v3, v4, tex, &tex->u1, &tex->u3, &tex->u4, sorttype);
        return;
    }
    if ((sint32)((v3->xs - v2->xs) * (v1->ys - v2->ys) - (v3->ys - v2->ys) * (v1->xs - v2->xs)) < 0)
        return;
    if ((sint8)(v1->clip | v2->clip | v3->clip | v4->clip)) {
        InsertGT3_Sorted(v1, v2, v3, tex, &tex->u1, &tex->u2, &tex->u3, sorttype);
        InsertGT3_Sorted(v1, v3, v4, tex, &tex->u1, &tex->u3, &tex->u4, sorttype);
        return;
    }
    switch (sorttype) {
    case MID_SORT:
        zdepth = (v1->zv + v2->zv + v3->zv + v4->zv) * 0.25f;
        break;
    case FAR_SORT:
        zdepth = v1->zv;
        if (zdepth < v2->zv)
    zdepth = v2->zv;
        if (zdepth < v3->zv)
    zdepth = v3->zv;
        if (zdepth < v4->zv)
    zdepth = v4->zv;
        break;
    default:
        zdepth = 1000000000.0f;
        break;
    }
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = (sint32)zdepth;
    sort3dptr += 2;
    info3dptr[0] = tex->drawtype + 9;
    info3dptr[1] = tex->tpage;
    info3dptr[2] = 4;
    vtx = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 3) = vtx;
    info3dptr += 5;
    vtx->sx = v1->xs;
    vtx->sy = v1->ys;
    vtx->rhw = v1->ooz;
    shade = (0x1fff - v1->g) >> 4;
    if (shade > 255)
        shade = 255;
    vtx->color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
    vtx->tu = tex->u1 / 65536.0f;
    vtx->tv = tex->v1 / 65536.0f;
    vtx++;
    vtx->sx = v2->xs;
    vtx->sy = v2->ys;
    vtx->rhw = v2->ooz;
    shade = (0x1fff - v2->g) >> 4;
    if (shade > 255)
        shade = 255;
    vtx->color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
    vtx->tu = tex->u2 / 65536.0f;
    vtx->tv = tex->v2 / 65536.0f;
    vtx++;
    vtx->sx = v3->xs;
    vtx->sy = v3->ys;
    vtx->rhw = v3->ooz;
    shade = (0x1fff - v3->g) >> 4;
    if (shade > 255)
        shade = 255;
    vtx->color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
    vtx->tu = tex->u3 / 65536.0f;
    vtx->tv = tex->v3 / 65536.0f;
    vtx++;
    vtx->sx = v4->xs;
    vtx->sy = v4->ys;
    vtx->rhw = v4->ooz;
    shade = (0x1fff - v4->g) >> 4;
    if (shade > 255)
        shade = 255;
    vtx->color = bBlueEffect ? 0xff000000 | (((shade * 128) >> 8) << 16) | (((shade * 224) >> 8) << 8) | shade : 0xff000000 | (shade << 16) | (shade << 8) | shade;
    vtx->tu = tex->u4 / 65536.0f;
    vtx->tv = tex->v4 / 65536.0f;
    if (SavedAppSettings.ZBuffer) {
        vtx[-3].sz = FltResZBuf - FltResZORhw * v1->ooz;
        vtx[-2].sz = FltResZBuf - FltResZORhw * v2->ooz;
        vtx[-1].sz = FltResZBuf - FltResZORhw * v3->ooz;
        vtx[0].sz = FltResZBuf - FltResZORhw * v4->ooz;
    }
    HWR_VertexPtr = vtx + 1;
    surfacenum++;
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

extern "C" void InsertPoly_Gouraud(int vtx_count, float z, int red, int green, int blue, int poly_type)
{
    int i, r, g, b;
    uint32 shade, alpha;
    bool zbuf;
    VERTEX_INFO *vbuf;
    D3DTLVERTEX *vtx;

    alpha = poly_type == 13 ? 0x80ffffff : 0xffffffff;
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = (sint32)z;
    sort3dptr += 2;
    info3dptr[0] = poly_type;
    info3dptr[1] = vtx_count;
    vtx = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 2) = vtx;
    info3dptr += 4;
    zbuf = SavedAppSettings.ZBuffer;
    vbuf = VBuffer;
    for (i = vtx_count; i; i--) {
        {
            vtx->sx = vbuf->x;
            vtx->sy = vbuf->y;
            if (zbuf)
                vtx->sz = FltResZBuf - FltResZORhw * vbuf->rhw;
            vtx->rhw = vbuf->rhw;
            shade = 0x1fff - (int)vbuf->g;
            r = (red * shade) >> 12;
            if (r > 255)
                r = 255;
            g = (green * shade) >> 12;
            if (g > 255)
                g = 255;
            b = (blue * shade) >> 12;
            if (b > 255)
                b = 255;
            vtx->color = ShadeColor(r, g, b) & alpha;
            vbuf++;
            vtx++;
        }
    }
    HWR_VertexPtr = vtx;
    surfacenum++;
}

extern "C" PALETTEENTRY GamePalette16[];
extern "C" int XYGClipper(int number, VERTEX_INFO *input);

extern "C" sint16 *InsertObjectG3_Sorted(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *vn1, *vn2, *vn3;
    POINT_INFO point[3];
    PALETTEENTRY *pal;
    int npoints;
    float zdepth;

    for (; number; number--, objptr += 4) {
        if (HWR_VertexBufferFull())
            return objptr + 4 * number;
        vn1 = &PhdVBuf[objptr[0]];
        vn2 = &PhdVBuf[objptr[1]];
        vn3 = &PhdVBuf[objptr[2]];
        if ((sint8)(vn1->clip & vn2->clip & vn3->clip))
            continue;
        if ((sint8)(vn1->clip | vn2->clip | vn3->clip) < 0) {
            if (!visible_zclip(vn1, vn2, vn3))
                continue;
            point[0].xv = vn1->xv;
            point[0].yv = vn1->yv;
            point[0].zv = vn1->zv;
            point[0].ooz = vn1->ooz;
            point[0].xs = vn1->xs;
            point[0].ys = vn1->ys;
            point[0].g = (float)vn1->g;
            point[1].xv = vn2->xv;
            point[1].yv = vn2->yv;
            point[1].zv = vn2->zv;
            point[1].ooz = vn2->ooz;
            point[1].xs = vn2->xs;
            point[1].ys = vn2->ys;
            point[1].g = (float)vn2->g;
            point[2].xv = vn3->xv;
            point[2].yv = vn3->yv;
            point[2].zv = vn3->zv;
            point[2].ooz = vn3->ooz;
            point[2].xs = vn3->xs;
            point[2].ys = vn3->ys;
            point[2].g = (float)vn3->g;
            npoints = ZedClipper(3, point, VBuffer);
            if (!npoints)
                continue;
            npoints = XYGClipper(npoints, VBuffer);
        } else {
            if ((sint32)((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys) - (vn3->ys - vn2->ys) * (vn1->xs - vn2->xs)) < 0)
                continue;
            VBuffer[0].x = vn1->xs;
            VBuffer[0].y = vn1->ys;
            VBuffer[0].rhw = vn1->ooz;
            VBuffer[0].g = (float)vn1->g;
            VBuffer[1].x = vn2->xs;
            VBuffer[1].y = vn2->ys;
            VBuffer[1].rhw = vn2->ooz;
            VBuffer[1].g = (float)vn2->g;
            VBuffer[2].x = vn3->xs;
            VBuffer[2].y = vn3->ys;
            VBuffer[2].rhw = vn3->ooz;
            VBuffer[2].g = (float)vn3->g;
            npoints = 3;
            if (vn1->clip | vn2->clip | vn3->clip)
                npoints = XYGClipper(3, VBuffer);
        }
        if (npoints) {
            pal = &GamePalette16[(uint16)objptr[3] >> 8];
            switch (sorttype) {
            case MID_SORT:
                zdepth = (vn1->zv + vn2->zv + vn3->zv) * 0.333333333333333333f;
                break;
            case FAR_SORT:
                zdepth = vn1->zv;
                if (vn2->zv > zdepth)
                    zdepth = vn2->zv;
                if (vn3->zv > zdepth)
                    zdepth = vn3->zv;
                break;
            default:
                zdepth = 1000000000.0f;
                break;
            }
            InsertPoly_Gouraud(npoints, zdepth, pal->peRed, pal->peGreen, pal->peBlue, 11);
        }
    }
    return objptr;
}

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
extern "C" int UvAdd;

extern "C" void InsertSprite_Sorted(int z, int x1, int y1, int x2, int y2, int sprnum, int shade)
{
    float zdepth, rhw, g, u1, v1, u2, v2, uvadd;
    PHDSPRITESTRUCT *sprite;
    int npoints;
    bool blue;

    if (HWR_VertexBufferFull())
        return;
    if (x2 <= x1 || y2 <= y1 || x2 <= 0 || y2 <= 0 || x1 >= phd_winxmax || y1 >= phd_winymax)
        return;
    x1 += phd_winxmin;
    x2 += phd_winxmin;
    y1 += phd_winymin;
    y2 += phd_winymin;
    if (z < phd_znear)
        z = phd_znear;
    if (z >= phd_zfar)
        return;
    zdepth = (float)z;
    rhw = RhwFactor / zdepth;
    g = (float)shade;
    sprite = &PhdSpriteInfo[sprnum];
    u1 = (float)((sprite->offset << 8) & 0xff00);
    v1 = (float)(sprite->offset & 0xff00);
    u2 = u1 + sprite->width;
    v2 = v1 + sprite->height;
    uvadd = (float)UvAdd;
    u1 += uvadd;
    u2 -= uvadd;
    v1 += uvadd;
    v2 -= uvadd;
    u1 *= rhw;
    v1 *= rhw;
    u2 *= rhw;
    v2 *= rhw;
    VBuffer[0].x = (float)x1;
    VBuffer[0].y = (float)y1;
    VBuffer[0].rhw = rhw;
    VBuffer[0].g = g;
    VBuffer[0].u = u1;
    VBuffer[0].v = v1;
    VBuffer[1].x = (float)x2;
    VBuffer[1].y = (float)y1;
    VBuffer[1].rhw = rhw;
    VBuffer[1].g = g;
    VBuffer[1].u = u2;
    VBuffer[1].v = v1;
    VBuffer[2].x = (float)x2;
    VBuffer[2].y = (float)y2;
    VBuffer[2].rhw = rhw;
    VBuffer[2].g = g;
    VBuffer[2].u = u2;
    VBuffer[2].v = v2;
    VBuffer[3].x = (float)x1;
    VBuffer[3].y = (float)y2;
    VBuffer[3].rhw = rhw;
    VBuffer[3].g = g;
    VBuffer[3].u = u1;
    VBuffer[3].v = v2;
    npoints = 4;
    if (x1 < phd_winxmin || x2 > phd_winxmin + phd_winwidth || y1 < phd_winymin || y2 > phd_winymin + phd_winheight) {
        phd_leftfloat = (float)phd_winxmin;
        phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
        phd_topfloat = (float)phd_winymin;
        phd_bottomfloat = (float)(phd_winymin + phd_winheight);
        npoints = XYGUVClipper(npoints, VBuffer);
    }
    if (npoints) {
        blue = bBlueEffect;
        bBlueEffect = false;
        InsertClippedPoly_Textured(npoints, zdepth, 10, sprite->tpage);
        bBlueEffect = blue;
    }
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

extern "C" void InsertLine_Sorted(int x1, int y1, int x2, int y2, int z, uint8 color)
{
    uint8 *rgb;
    D3DTLVERTEX *v;

    rgb = (uint8 *)game_palette + color * 3;
    sort3dptr[0] = (sint32)info3dptr;
    sort3dptr[1] = z;
    sort3dptr += 2;
    info3dptr[0] = 12;
    info3dptr[1] = 2;
    v = HWR_VertexPtr;
    *(D3DTLVERTEX **)(info3dptr + 2) = v;
    info3dptr += 4;
    v[0].sx = (float)(x1 + phd_winxmin);
    v[0].sy = (float)(y1 + phd_winymin);
    v[1].sx = (float)(x2 + phd_winxmin);
    v[1].sy = (float)(y2 + phd_winymin);
    v[0].color = v[1].color = ShadeColor(rgb[0], rgb[1], rgb[2]);
    HWR_VertexPtr = v + 2;
    surfacenum++;
}

extern "C" int XYClipper(int number, VERTEX_INFO *input);

extern "C" void InsertTrans8_Sorted(PHD_VBUF *vbuf, sint16 shade)
{
    int i, n;
    sint8 or_clip;

    or_clip = (sint8)vbuf[0].clip | (sint8)vbuf[1].clip | (sint8)vbuf[2].clip | (sint8)vbuf[3].clip | (sint8)vbuf[4].clip | (sint8)vbuf[5].clip | (sint8)vbuf[6].clip | (sint8)vbuf[7].clip;
    if (or_clip < 0)
        return;
    if ((sint8)vbuf[0].clip & (sint8)vbuf[1].clip & (sint8)vbuf[2].clip & (sint8)vbuf[3].clip & (sint8)vbuf[4].clip & (sint8)vbuf[5].clip & (sint8)vbuf[6].clip & (sint8)vbuf[7].clip)
        return;
    if ((sint32)((vbuf[2].xs - vbuf[1].xs) * (vbuf[0].ys - vbuf[1].ys) - (vbuf[0].xs - vbuf[1].xs) * (vbuf[2].ys - vbuf[1].ys)) < 0)
        return;
    for (i = 0; i < 8; i++)
    {
        VBuffer[i].x = vbuf[i].xs;
        VBuffer[i].y = vbuf[i].ys;
        VBuffer[i].rhw = RhwFactor / (vbuf[i].zv - 131072);
    }
    n = 8;
    if (or_clip)
    {
        phd_leftfloat = (float)phd_winxmin;
        phd_topfloat = (float)phd_winymin;
        phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
        phd_bottomfloat = (float)(phd_winymin + phd_winheight);
        n = XYClipper(n, VBuffer);
    }
    if (n)
        InsertPoly_Gouraud(n, (vbuf[0].zv + vbuf[1].zv + vbuf[2].zv + vbuf[3].zv + vbuf[4].zv + vbuf[5].zv + vbuf[6].zv + vbuf[7].zv) * 0.125f - 131072, 0, 0, 0, 13);
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
