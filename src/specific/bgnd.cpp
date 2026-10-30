#include "include.h"
#include "global.h"
#include <math.h>

extern "C" {
extern bool BGND_PictureIsReady;
void BGND_Free(void);
extern int BGND_TexturePageIndexes[5];
extern DWORD BGND_PageHandles[5];
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int phd_winwidth;
extern int phd_winheight;
}

extern "C" DWORD GetTexturePageHandle(int nHandle);
extern "C" void DrawQuad(float sx, float sy, float width, float height, D3DCOLOR color);
void HWR_EnableZBuffer(bool tWrite, bool tCompare);

extern "C" void BGND_GetPageHandles(void)
{
    DWORD i;

    for (i = 0; i < 5; i++)
    {
        if (BGND_TexturePageIndexes[i] >= 0)
            BGND_PageHandles[i] = GetTexturePageHandle(BGND_TexturePageIndexes[i]);
        else
            BGND_PageHandles[i] = 0;
    }
}

extern "C" float FltRhwONearZ;
void HWR_TexSource(D3DTEXTUREHANDLE tSource);
void HWR_EnableColorKey(bool tEnable);

extern "C" void DrawQuad(float sx, float sy, float width, float height, D3DCOLOR color)
{
    D3DTLVERTEX vertex[4];
    int i;

    vertex[0].sx = sx;
    vertex[0].sy = sy;
    vertex[1].sx = sx + width;
    vertex[1].sy = sy;
    vertex[2].sx = sx;
    vertex[2].sy = sy + height;
    vertex[3].sx = sx + width;
    vertex[3].sy = sy + height;
    for (i = 0; i < 4; i++) {
        vertex[i].sz = 0.0f;
        vertex[i].rhw = FltRhwONearZ;
        vertex[i].color = RGBA_SETALPHA(color, 0xFF);
        vertex[i].specular = 0;
    }
    HWR_TexSource(0);
    HWR_EnableColorKey(false);
    G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, vertex, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
}

extern "C" void BGND_DrawInGameBlack(void)
{
    HWR_EnableZBuffer(false, false);
    DrawQuad((float)phd_winxmin, (float)phd_winymin, (float)phd_winwidth, (float)phd_winheight, 0);
    HWR_EnableZBuffer(true, true);
}

extern "C" void Func444560(void)
{
    BGND_Free();
    BGND_PictureIsReady = false;
}

extern "C" int BGND_PaletteIndex;
extern "C" void SafeFreeTexturePage(int pageIndex);
extern "C" void SafeFreePalette(int palIndex);

extern "C" void BGND_Free(void)
{
    DWORD i;

    for (i = 0; i < 5; i++) {
        if (BGND_TexturePageIndexes[i] >= 0) {
            SafeFreeTexturePage(BGND_TexturePageIndexes[i]);
            BGND_TexturePageIndexes[i] = -1;
        }
        BGND_PageHandles[i] = 0;
    }
    if (BGND_PaletteIndex >= 0) {
        SafeFreePalette(BGND_PaletteIndex);
        BGND_PaletteIndex = -1;
    }
}

extern "C" D3DCOLOR BGND_CenterLighting(int x, int y, int width, int height)
{
    double xDist = (double)(x - (width >> 1)) / (double)width;
    double yDist = (double)(y - (height >> 1)) / (double)height;
    int light = 256 - (int)(sqrt(xDist * xDist + yDist * yDist) * 300.0);

    if (light < 0)
        light = 0;
    else if (light > 255)
        light = 255;
    return RGBA_MAKE(light, light, light, 0xFF);
}
