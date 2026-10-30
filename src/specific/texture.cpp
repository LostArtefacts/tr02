#include "include.h"
#include "global.h"

#define MAX_PALETTES 16
#define MAX_TEXTURE_PAGES 32

typedef struct {
    LPDIRECTDRAWSURFACE3 vidMemSurface;
    LPDIRECTDRAWSURFACE3 sysMemSurface;
    LPDIRECT3DTEXTURE2 texture3d;
    D3DTEXTUREHANDLE texHandle;
    int width;
    int height;
    int palIndex;
    DWORD status;
} TEXPAGE_DESC;

extern "C" {
extern TEXPAGE_DESC TexturePages[MAX_TEXTURE_PAGES];
extern LPDIRECTDRAWPALETTE TexturePalettes[MAX_PALETTES];
}

extern "C" int GetFreePaletteIndex(void)
{
    for (DWORD i = 0; i < MAX_PALETTES; i++)
    {
        if (!TexturePalettes[i])
            return i;
    }
    return -1;
}

extern "C" void FreePalette(int palIndex)
{
    if (TexturePalettes[palIndex])
    {
        TexturePalettes[palIndex]->Release();
        TexturePalettes[palIndex] = NULL;
    }
}
