#include <ddraw.h>
#include "global.h"

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

extern "C" HRESULT Func455940(void)
{
    DDSURFACEDESC desc;
    HRESULT result;

    result = WinVidBufferLock(RenderBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT);
    if (DX_TRY(result))
        return result;
    phd_PrintPolyList(desc.lpSurface);
    return WinVidBufferUnlock(RenderBufferSurface, &desc);
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
