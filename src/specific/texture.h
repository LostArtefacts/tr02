#ifndef TEXTURE_H
#define TEXTURE_H

#include "../types.h"
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

#define MAX_PALETTES 16
#define MAX_TEXTURE_PAGES 32

typedef struct {
    LPDIRECTDRAWSURFACE3 sysMemSurface;
    LPDIRECTDRAWSURFACE3 vidMemSurface;
    LPDIRECTDRAWPALETTE palette;
    LPDIRECT3DTEXTURE2 texture3d;
    D3DTEXTUREHANDLE texHandle;
    int width;
    int height;
    DWORD status;
} TEXPAGE_DESC;

typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
} RGB888;

#ifdef __cplusplus
extern "C" {
#endif

extern TEXPAGE_DESC TexturePages[MAX_TEXTURE_PAGES];
extern LPDIRECTDRAWPALETTE TexturePalettes[MAX_PALETTES];
extern bool TexturesAlphaChannel;
extern bool TexturesHaveCompatibleMasks;

int CreateTexturePalette(RGB888 *pal);
int GetFreePaletteIndex(void);
void FreePalette(int palIndex);
void SafeFreePalette(int palIndex);
void FreeTexturePalettes(void);
int CreateTexturePage(int width, int height, LPDIRECTDRAWPALETTE palette);
int GetFreeTexturePageIndex(void);
void SafeFreeTexturePage(int pageIndex);
void FreeTexturePage(int pageIndex);
void FreeTexturePages(void);
bool LoadTexturePage(int pageIndex, bool reset);
bool ReloadTextures(bool reset);
D3DTEXTUREHANDLE GetTexturePageHandle(int pageIndex);
int AddTexturePage8(int width, int height, uint8 *pageBuffer, int palIndex);
int AddTexturePage16(int width, int height, uint8 *pageBuffer);
void EnumerateTextureFormats(void);
void CleanupTextures(void);
bool InitTextures(void);

#ifdef __cplusplus
}
#endif

#endif
