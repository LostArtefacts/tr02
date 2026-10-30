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

typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
} RGB888;

typedef struct {
    uint8 pad[0x50];
    int bpp;
} BGND_TEXTURE_FORMAT;

extern "C" {
extern BGND_TEXTURE_FORMAT TextureFormat;
extern int BGND_PaletteIndex;
void *game_malloc(int size, int type);
void game_free(int size, int type);
int CreateTexturePalette(RGB888 *pal);
int BGND_AddTexture(int tileIndex, uint8 *bitmap, int palIndex, RGB888 *bmpPal);
void BGND_GetPageHandles(void);
}

void UT_MemBlt(uint8 *pDest, int nDX, int nDY, int nDW, int nDH, int nDStride, uint8 *pSrc, int nSX, int nSY, int nSStride);

extern "C" void BGND_Make640x480(uint8 *bitmap, RGB888 *palette)
{
    uint8 *buffer;

    if (TextureFormat.bpp < 16)
        BGND_PaletteIndex = CreateTexturePalette(palette);
    else
        BGND_PaletteIndex = -1;
    buffer = (uint8 *)game_malloc(0x20000, 0);
    UT_MemBlt(buffer, 0, 0, 256, 256, 256, bitmap, 0, 0, 640);
    BGND_AddTexture(0, buffer, BGND_PaletteIndex, palette);
    UT_MemBlt(buffer, 0, 0, 256, 256, 256, bitmap, 256, 0, 640);
    BGND_AddTexture(1, buffer, BGND_PaletteIndex, palette);
    UT_MemBlt(buffer, 0, 0, 128, 256, 256, bitmap, 512, 0, 640);
    UT_MemBlt(buffer, 128, 0, 128, 224, 256, bitmap, 512, 256, 640);
    BGND_AddTexture(2, buffer, BGND_PaletteIndex, palette);
    UT_MemBlt(buffer, 0, 0, 256, 224, 256, bitmap, 0, 256, 640);
    BGND_AddTexture(3, buffer, BGND_PaletteIndex, palette);
    UT_MemBlt(buffer, 0, 0, 256, 224, 256, bitmap, 256, 256, 640);
    BGND_AddTexture(4, buffer, BGND_PaletteIndex, palette);
    game_free(0x20000, 0);
    BGND_GetPageHandles();
    BGND_PictureIsReady = true;
}

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

extern "C" {
extern int UvAdd;
extern float RhwFactor;
extern float FltFarZ;
}

extern "C" void DrawTextureTile(int sx, int sy, int width, int height, D3DTEXTUREHANDLE texSource, int tu, int tv, int tWidth, int tHeight, D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
    D3DTLVERTEX vertex[4];
    float sx0;
    float sy0;
    float sx1;
    float sy1;
    float tu0;
    float tv0;
    float tu1;
    float tv1;
    float sz;
    float rhw;
    float uvAdjust;

    sx0 = (float)sx;
    sy0 = (float)sy;
    sx1 = (float)(sx + width);
    sy1 = (float)(sy + height);
    tu0 = (float)tu * (1.0f / 256.0f);
    tv0 = (float)tv * (1.0f / 256.0f);
    tu1 = (float)(tu + tWidth) * (1.0f / 256.0f);
    tv1 = (float)(tv + tHeight) * (1.0f / 256.0f);
    uvAdjust = (float)UvAdd * (1.0f / 65536.0f);
    tu0 += uvAdjust;
    tv0 += uvAdjust;
    tu1 -= uvAdjust;
    tv1 -= uvAdjust;
    sz = 0.995f;
    rhw = RhwFactor / FltFarZ;
    vertex[0].sx = sx0;
    vertex[0].sy = sy0;
    vertex[0].sz = sz;
    vertex[0].rhw = rhw;
    vertex[0].color = color0;
    vertex[0].specular = 0;
    vertex[0].tu = tu0;
    vertex[0].tv = tv0;
    vertex[1].sx = sx1;
    vertex[1].sy = sy0;
    vertex[1].sz = sz;
    vertex[1].rhw = rhw;
    vertex[1].color = color1;
    vertex[1].specular = 0;
    vertex[1].tu = tu1;
    vertex[1].tv = tv0;
    vertex[2].sx = sx0;
    vertex[2].sy = sy1;
    vertex[2].sz = sz;
    vertex[2].rhw = rhw;
    vertex[2].color = color2;
    vertex[2].specular = 0;
    vertex[2].tu = tu0;
    vertex[2].tv = tv1;
    vertex[3].sx = sx1;
    vertex[3].sy = sy1;
    vertex[3].sz = sz;
    vertex[3].rhw = rhw;
    vertex[3].color = color3;
    vertex[3].specular = 0;
    vertex[3].tu = tu1;
    vertex[3].tv = tv1;
    HWR_TexSource(texSource);
    HWR_EnableColorKey(false);
    G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, vertex, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
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
