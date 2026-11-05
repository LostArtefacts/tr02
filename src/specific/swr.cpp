#include "standard.h"
#include "global.h"
#include "texture.h"

inline bool DX_TRY(HRESULT hr)
{
    return FAILED(hr);
}

extern "C" {
extern LPDIRECTDRAWSURFACE RenderBufferSurface;
HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE surface, LPDDSURFACEDESC desc, DWORD flags);
HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE surface, LPDDSURFACEDESC desc);
void phd_PrintPolyList(void *ptr);
}

extern "C" HRESULT SWR_DrawPolyList(void)
{
    DDSURFACEDESC desc;
    HRESULT result;

    result = WinVidBufferLock(RenderBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT);
    if (DX_TRY(result))
        return result;
    phd_PrintPolyList(desc.lpSurface);
    return WinVidBufferUnlock(RenderBufferSurface, &desc);
}

extern "C" sint32 sort3d_buffer[][2];
extern "C" void do_quickysorty(int left, int right);

extern "C" void CopyBitmapPalette(RGB888 *srcPal, uint8 *srcBitmap, int bitmapSize, RGB888 *destPal)
{
    int i;
    HDC hdc;
    RGB888 *src;
    RGB888 *dst;
    PALETTEENTRY sys[20];

    for (i = 0; i < 256; i++)
    {
        sort3d_buffer[i][0] = i;
        sort3d_buffer[i][1] = 0;
    }
    while (bitmapSize--)
        sort3d_buffer[*srcBitmap++][1]++;
    do_quickysorty(0, 255);
    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0, 10, sys);
    GetSystemPaletteEntries(hdc, 246, 10, sys + 10);
    ReleaseDC(NULL, hdc);
    dst = destPal;
    for (i = 0; i < 8; i++, dst++)
    {
        dst->red = sys[i].peRed;
        dst->green = sys[i].peGreen;
        dst->blue = sys[i].peBlue;
    }
    destPal[8].red = destPal[9].red = destPal[246].red = destPal[8].green = destPal[9].green = destPal[246].green = destPal[8].blue = destPal[9].blue = destPal[246].blue = 0;
    dst = destPal + 247;
    for (i = 0; i < 9; i++, dst++)
    {
        dst->red = sys[i + 11].peRed;
        dst->green = sys[i + 11].peGreen;
        dst->blue = sys[i + 11].peBlue;
    }
    dst = destPal + 10;
    for (i = 0; i < 236; i++, dst++)
    {
        src = &srcPal[sort3d_buffer[i][0]];
        dst->red = src->red;
        dst->green = src->green;
        dst->blue = src->blue;
    }
}

extern "C" uint8 SWR_FindNearestPaletteEntry(uint8 *pPal, int nR, int nG, int nB, bool tDontUseWindowsColours)
{
    int i, nEnd, nColour;
    double dR, dG, dB;
    double dDist, dClosest;

    dClosest = 10000000000.0;
    if (tDontUseWindowsColours)
    {
        i = 10;
        nEnd = 246;
        pPal += 10 * 3;
    }
    else
    {
        i = 0;
        nEnd = 256;
    }
    for (; i < nEnd; ++i)
    {
        dR = double(nR - *(pPal++));
        dG = double(nG - *(pPal++));
        dB = double(nB - *(pPal++));
        dDist = dR * dR + dG * dG + dB * dB;
        if (dDist < dClosest)
        {
            nColour = i;
            dClosest = dDist;
        }
    }
    return (uint8)nColour;
}

extern "C" void SyncSurfacePalettes(uint8 *src, uint32 width, uint32 height, int srcPitch, RGB888 *srcPalette, uint8 *dst, int dstPitch, RGB888 *dstPalette, bool preserveSysPalette)
{
    int i;
    uint32 x;
    uint8 bitmap[256];

    for (i = 0; i < 256; i++)
        bitmap[i] = SWR_FindNearestPaletteEntry((uint8 *)dstPalette, srcPalette[i].red, srcPalette[i].green, srcPalette[i].blue, preserveSysPalette);
    while (height--)
    {
        x = width;
        while (x--)
            *dst++ = bitmap[*src++];
        src += srcPitch - width;
        dst += dstPitch - width;
    }
}
