#include "include.h"
#include "global.h"
#include "texture.h"

struct COLOR_BIT_MASKS {
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwRGBAlphaBitMask;
    DWORD dwRBitDepth;
    DWORD dwGBitDepth;
    DWORD dwBBitDepth;
    DWORD dwRGBAlphaBitDepth;
    DWORD dwRBitOffset;
    DWORD dwGBitOffset;
    DWORD dwBBitOffset;
    DWORD dwRGBAlphaBitOffset;
    void WinVidGetColorBitMasks(LPDDPIXELFORMAT pixelFormat);
};

typedef struct {
    DDPIXELFORMAT pixelFormat;
    COLOR_BIT_MASKS colorBitMasks;
    DWORD bpp;
} TEXTURE_FORMAT;

typedef struct {
    uint8 pad[0x29];
    bool Disable16BitTextures;
} TEXTURE_APP_SETTINGS;

extern "C" {
extern TEXTURE_APP_SETTINGS SavedAppSettings;
extern TEXTURE_FORMAT TextureFormat;
}

extern "C" LPDIRECTDRAW2 DDraw;


extern "C" int CreateTexturePalette(RGB888 *pal)
{
    PALETTEENTRY entries[256];
    int index;
    int i;

    index = GetFreePaletteIndex();
    if (index < 0)
        return -1;
    for (i = 0; i < 256; i++, pal++)
        *(DWORD *)&entries[i] = pal->red | (pal->green << 8) | (pal->blue << 16);
    if (FAILED(DDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, entries, &TexturePalettes[index], NULL)))
        return -1;
    return index;
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
    desc.ddpfPixelFormat = TextureFormat.pixelFormat;
    if (DX_TRY(DDrawSurfaceCreate(&desc, &page->sysMemSurface)))
        return false;
    if (page->palette && DX_TRY(page->sysMemSurface->SetPalette(page->palette)))
        return false;
    return true;
}

extern "C" LPDIRECT3DTEXTURE2 Create3DTexture(LPDIRECTDRAWSURFACE3 surface);

extern "C" bool TexturePageInit(TEXPAGE_DESC *page)
{
    DDSURFACEDESC desc;
    DDCOLORKEY colorKey;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD;
    desc.dwWidth = page->width;
    desc.dwHeight = page->height;
    desc.ddpfPixelFormat = TextureFormat.pixelFormat;
    if (DX_TRY(DDrawSurfaceCreate(&desc, &page->vidMemSurface)))
        return false;
    if (page->palette)
    {
        if (DX_TRY(page->vidMemSurface->SetPalette(page->palette)))
        {
            if (page->vidMemSurface)
            {
                page->vidMemSurface->Release();
                page->vidMemSurface = NULL;
            }
            return false;
        }
        colorKey.dwColorSpaceLowValue = 0;
        colorKey.dwColorSpaceHighValue = 0;
        if (DX_TRY(page->vidMemSurface->SetColorKey(DDCKEY_SRCBLT, &colorKey)))
        {
            if (page->vidMemSurface)
            {
                page->vidMemSurface->Release();
                page->vidMemSurface = NULL;
            }
            return false;
        }
    }
    page->texture3d = Create3DTexture(page->vidMemSurface);
    if (!page->texture3d)
    {
        if (page->vidMemSurface)
        {
            page->vidMemSurface->Release();
            page->vidMemSurface = NULL;
        }
        return false;
    }
    if (DX_TRY(page->texture3d->GetHandle(G_Direct3DDevice, &page->texHandle)))
    {
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
        page->texHandle = 0;
        return false;
    }
    return true;
}

extern "C" LPDIRECT3DTEXTURE2 Create3DTexture(LPDIRECTDRAWSURFACE3 surface)
{
    LPDIRECT3DTEXTURE2 texture;
    if (DX_TRY(surface->QueryInterface(IID_IDirect3DTexture2, (void **)&texture)))
        return NULL;
    return texture;
}


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

extern "C" HRESULT DDrawSurfaceRestoreLost(LPDIRECTDRAWSURFACE3 surface1, LPDIRECTDRAWSURFACE3 surface2, bool blank);

extern "C" bool LoadTexturePage(int pageIndex, bool reset)
{
    bool rc;
    TEXPAGE_DESC *page;
    LPDIRECT3DTEXTURE2 texture;

    if (pageIndex < 0)
        return false;
    page = &TexturePages[pageIndex];
    if (!reset)
    {
        if (!page->vidMemSurface)
            goto texture_reset;
        if (SUCCEEDED(DDrawSurfaceRestoreLost(page->vidMemSurface, NULL, false)))
            goto texture_restored;
        reset = true;
    }
    if (reset)
    {
texture_reset:
        TexturePageReleaseVidMemSurface(page);
        if (!TexturePageInit(page))
            return false;
    }
texture_restored:
    DDrawSurfaceRestoreLost(page->sysMemSurface, NULL, false);
    texture = Create3DTexture(page->sysMemSurface);
    if (!texture)
        return false;
    rc = !DX_TRY(page->texture3d->Load(texture));
    texture->Release();
    return rc;
}

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

extern "C" D3DTEXTUREHANDLE GetTexturePageHandle(int pageIndex)
{
    if (pageIndex < 0)
        return 0;
    TEXPAGE_DESC *page = &TexturePages[pageIndex];
    if (page->vidMemSurface && page->vidMemSurface->IsLost() == DDERR_SURFACELOST)
        LoadTexturePage(pageIndex, true);
    return page->texHandle;
}

extern "C" HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE3 surface, LPDDSURFACEDESC desc, DWORD flags);
extern "C" HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE3 surface, LPDDSURFACEDESC desc);

inline bool DX_FAILED(HRESULT hr)
{
    return FAILED(hr);
}

extern "C" int AddTexturePage8(int width, int height, uint8 *pageBuffer, int palIndex)
{
    int pageIndex;
    TEXPAGE_DESC *page;
    DDSURFACEDESC desc;
    uint8 *dst;
    DWORD i;
    int pitch;
    if (palIndex < 0)
        return -1;
    pageIndex = Func455d80(width, height, TexturePalettes[palIndex]);
    if (pageIndex < 0)
        return -1;
    page = &TexturePages[pageIndex];
    if (DX_FAILED(WinVidBufferLock(page->sysMemSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
        return -1;
    dst = (uint8 *)desc.lpSurface;
    pitch = desc.lPitch;
    for (i = height; i != 0; i--)
    {
        memcpy(dst, pageBuffer, width);
        pageBuffer += width;
        dst += pitch;
    }
    WinVidBufferUnlock(page->sysMemSurface, &desc);
    LoadTexturePage(pageIndex, false);
    return pageIndex;
}

extern "C" DWORD CalculateCompatibleColor(COLOR_BIT_MASKS *mask, int red, int green, int blue, int alpha);

extern "C" int AddTexturePage16(int width, int height, uint8 *pageBuffer)
{
    int pageIndex;
    TEXPAGE_DESC *page;
    DDSURFACEDESC desc;
    uint8 *dst;
    uint8 *line;
    uint16 color;
    DWORD compatible;
    DWORD i, j;
    int k;
    int pitch;

    pageIndex = Func455d80(width, height, NULL);
    if (pageIndex < 0)
        return -1;
    page = &TexturePages[pageIndex];
    if (DX_FAILED(WinVidBufferLock(page->sysMemSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
        return -1;
    pitch = desc.lPitch;
    if (TexturesHaveCompatibleMasks)
    {
        dst = (uint8 *)desc.lpSurface;
        for (i = height; i != 0; i--)
        {
            memcpy(dst, pageBuffer, width << 1);
            pageBuffer += width * 2;
            dst += pitch;
        }
    }
    else
    {
        dst = (uint8 *)desc.lpSurface;
        for (i = height; i != 0; i--)
        {
            line = dst;
            for (j = width; j--;)
            {
                color = *(uint16 *)pageBuffer;
                pageBuffer += 2;
                compatible = CalculateCompatibleColor(&TextureFormat.colorBitMasks, (color >> 7) & 0xf8, (color >> 2) & 0xf8, (color & 0x1f) << 3, (sint16)color >> 15);
                for (k = TextureFormat.bpp; k > 0; k -= 8)
                {
                    *line++ = (uint8)compatible;
                    compatible >>= 8;
                }
            }
            dst += pitch;
        }
    }
    WinVidBufferUnlock(page->sysMemSurface, &desc);
    LoadTexturePage(pageIndex, false);
    return pageIndex;
}

extern "C" HRESULT CALLBACK EnumTextureFormatsCallback(LPDDSURFACEDESC desc, LPVOID context);

extern "C" HRESULT CALLBACK EnumTextureFormatsCallback(LPDDSURFACEDESC desc, LPVOID context)
{
    DDPIXELFORMAT pixelFormat = desc->ddpfPixelFormat;
    if (pixelFormat.dwRGBBitCount < 8)
        return D3DENUMRET_OK;
    if (!SavedAppSettings.Disable16BitTextures && pixelFormat.dwRGBBitCount == 16)
    {
        if (!(pixelFormat.dwFlags & DDPF_RGB))
            return D3DENUMRET_OK;
        TextureFormat.pixelFormat = desc->ddpfPixelFormat;
        TextureFormat.bpp = 16;
        TexturesAlphaChannel = pixelFormat.dwFlags & DDPF_ALPHAPIXELS;
        TextureFormat.colorBitMasks.WinVidGetColorBitMasks(&pixelFormat);
        if (TextureFormat.bpp == 16
            && TextureFormat.colorBitMasks.dwRGBAlphaBitDepth == 1
            && TextureFormat.colorBitMasks.dwRBitDepth == 5
            && TextureFormat.colorBitMasks.dwGBitDepth == 5
            && TextureFormat.colorBitMasks.dwBBitDepth == 5
            && TextureFormat.colorBitMasks.dwRGBAlphaBitOffset == 15
            && TextureFormat.colorBitMasks.dwRBitOffset == 10
            && TextureFormat.colorBitMasks.dwGBitOffset == 5
            && TextureFormat.colorBitMasks.dwBBitOffset == 0)
        {
            TexturesHaveCompatibleMasks = true;
            return D3DENUMRET_CANCEL;
        }
    }
    else
    {
        if (!(pixelFormat.dwFlags & DDPF_PALETTEINDEXED8))
            return D3DENUMRET_OK;
        TextureFormat.pixelFormat = desc->ddpfPixelFormat;
        TextureFormat.bpp = 8;
        TexturesAlphaChannel = false;
    }
    TexturesHaveCompatibleMasks = false;
    return D3DENUMRET_OK;
}

extern "C" void EnumerateTextureFormats(void)
{
    memset(&TextureFormat, 0, 86);
    G_Direct3DDevice->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
}

extern "C" void CleanupTextures(void)
{
    FreeTexturePages();
    Func455d50();
}

extern "C" bool InitTextures(void)
{
    memset(TexturePages, 0, sizeof(TexturePages));
    memset(TexturePalettes, 0, sizeof(TexturePalettes));
    return true;
}

#ifdef TR2_EXE
TEXPAGE_DESC TexturePages[MAX_TEXTURE_PAGES];
TEXTURE_FORMAT TextureFormat;
bool TexturesAlphaChannel;
bool TexturesHaveCompatibleMasks;
LPDIRECTDRAWPALETTE TexturePalettes[MAX_PALETTES];
#endif
