#include "include.h"
#include "global.h"

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

extern "C" {
extern TEXPAGE_DESC TexturePages[MAX_TEXTURE_PAGES];
extern LPDIRECTDRAWPALETTE TexturePalettes[MAX_PALETTES];
extern DDPIXELFORMAT TextureFormat;
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

extern "C" void SafeFreePalette(int palIndex)
{
    if (palIndex >= 0)
        FreePalette(palIndex);
}

extern "C" void Func455d50(void)
{
    for (DWORD i = 0; i < MAX_PALETTES; i++)
    {
        if (TexturePalettes[i])
            FreePalette(i);
    }
}

extern "C" int GetFreeTexturePageIndex(void);
extern "C" bool CreateTexturePageSurface(TEXPAGE_DESC *page);
extern "C" bool TexturePageInit(TEXPAGE_DESC *page);

extern "C" int Func455d80(int width, int height, LPDIRECTDRAWPALETTE palette)
{
    int pageIndex = GetFreeTexturePageIndex();
    if (pageIndex < 0)
        return -1;
    TEXPAGE_DESC *page = &TexturePages[pageIndex];
    memset(page, 0, sizeof(TEXPAGE_DESC));
    page->status = 1;
    page->width = width;
    page->height = height;
    page->palette = palette;
    if (!CreateTexturePageSurface(page))
        return -1;
    TexturePageInit(page);
    return pageIndex;
}

extern "C" int GetFreeTexturePageIndex(void)
{
    for (DWORD i = 0; i < MAX_TEXTURE_PAGES; i++)
    {
        if (!(TexturePages[i].status & 1))
            return i;
    }
    return -1;
}

extern "C" HRESULT DDrawSurfaceCreate(DDSURFACEDESC *desc, LPDIRECTDRAWSURFACE3 *surface);

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" bool CreateTexturePageSurface(TEXPAGE_DESC *page)
{
    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
    desc.dwWidth = page->width;
    desc.dwHeight = page->height;
    desc.ddpfPixelFormat = TextureFormat;
    if (DX_TRY(DDrawSurfaceCreate(&desc, &page->sysMemSurface)))
        return false;
    if (page->palette && DX_TRY(page->sysMemSurface->SetPalette(page->palette)))
        return false;
    return true;
}

extern "C" LPDIRECT3DTEXTURE2 Create3DTexture(LPDIRECTDRAWSURFACE3 surface)
{
    LPDIRECT3DTEXTURE2 texture;
    if (DX_TRY(surface->QueryInterface(IID_IDirect3DTexture2, (void **)&texture)))
        return NULL;
    return texture;
}

extern "C" void FreeTexturePage(int pageIndex);

extern "C" void SafeFreeTexturePage(int pageIndex)
{
    if (pageIndex >= 0 && (TexturePages[pageIndex].status & 1))
        FreeTexturePage(pageIndex);
}

extern "C" void TexturePageReleaseVidMemSurface(TEXPAGE_DESC *page);

extern "C" void FreeTexturePage(int pageIndex)
{
    TEXPAGE_DESC *page = &TexturePages[pageIndex];
    TexturePageReleaseVidMemSurface(page);
    if (page->sysMemSurface)
    {
        page->sysMemSurface->Release();
        page->sysMemSurface = NULL;
    }
    page->status = 0;
}

extern "C" void HWR_ResetTexSource(void);

extern "C" void TexturePageReleaseVidMemSurface(TEXPAGE_DESC *page)
{
    HWR_ResetTexSource();
    page->texHandle = 0;
    if (page->texture3d)
    {
        page->texture3d->Release();
        page->texture3d = NULL;
    }
    if (page->vidMemSurface)
    {
        page->vidMemSurface->Release();
        page->vidMemSurface = NULL;
    }
}

extern "C" void FreeTexturePages(void)
{
    for (DWORD i = 0; i < MAX_TEXTURE_PAGES; i++)
    {
        if (TexturePages[i].status & 1)
            FreeTexturePage(i);
    }
}

extern "C" bool LoadTexturePage(int pageIndex, bool reset);

extern "C" bool ReloadTextures(bool reset)
{
    bool result = true;
    for (DWORD i = 0; i < MAX_TEXTURE_PAGES; i++)
    {
        if (TexturePages[i].status & 1)
            result &= LoadTexturePage(i, reset);
    }
    return result;
}
