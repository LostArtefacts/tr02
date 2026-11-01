#include "include.h"
#include "global.h"
#include "hwrender.h"
#include "winmain.h"

#define MAX_TLVERTICES 8192

extern "C" D3DTLVERTEX VertexBuffer[MAX_TLVERTICES];
extern "C" int LanTextureHandle[32];
extern "C" bool ZEnableState;
extern "C" bool ZWriteEnableState;
extern "C" LPDIRECTDRAWSURFACE ZBufferSurface;
extern "C" DWORD HWR_PageHandles[32];
extern "C" DWORD GetTexturePageHandle(int nHandle);
extern "C" void SafeFreeTexturePage(int nHandle);
extern "C" void SafeFreePalette(int nHandle);
extern "C" int PaletteIndex;
extern "C" D3DTLVERTEX *HWR_VertexPtr;
extern "C" int CreateTexturePalette(uint8 *pPalette);
extern "C" int AddTexturePage8(int nWidth, int nHeight, uint8 *pTexture, int nPalette);
extern "C" int AddTexturePage16(int nWidth, int nHeight, uint8 *pTexture);
extern "C" bool ColorKeyState;
extern "C" bool TexturesAlphaChannel;
extern "C" D3DTEXTUREHANDLE CurrentTexSource;

typedef struct {
    LPTSTR lpString;
    bool isPresented;
} STRING_FLAGGED;

typedef struct {
    int width;
    int height;
    int bpp;
    int vga;
} DISPLAY_MODE;

typedef struct {
    void *head;
    void *tail;
    DWORD dwCount;
} DISPLAY_MODE_LIST;

typedef struct {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED driverDescription;
    STRING_FLAGGED driverName;
    DDCAPS driverCaps;
    DDCAPS helCaps;
    GUID deviceGuid;
    D3DDEVICEDESC D3DHWDeviceDesc;
    DISPLAY_MODE_LIST hwDispModeList;
    DISPLAY_MODE_LIST swDispModeList;
    DISPLAY_MODE vgaMode1;
    DISPLAY_MODE vgaMode2;
    DWORD screenWidth;
    bool hwRenderSupported;
    bool swWindowedSupported;
    bool hwWindowedSupported;
    bool isVgaMode1Presented;
    bool isVgaMode2Presented;
    bool perspectiveCorrectSupported;
    bool ditherSupported;
    bool zBufferSupported;
    bool linearFilterSupported;
    bool shadeRestricted;
} DISPLAY_ADAPTER;

typedef struct {
    APP_SETTINGS base;
    bool BilinearFiltering;
} APP_SETTINGS_EXT;

extern "C" DISPLAY_ADAPTER CurrentDisplayAdapter;
extern "C" D3DRENDERSTATETYPE AlphaBlendEnabler;
extern "C" void HWR_ResetTexSource(void);
void HWR_ResetColorKey();
void HWR_ResetZBuffer();

extern "C" void HWR_InitState(void)
{
    DWORD filter;

    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
    filter = ((APP_SETTINGS_EXT *)&SavedAppSettings)->BilinearFiltering ? D3DFILTER_LINEAR : D3DFILTER_NEAREST;
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, filter);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, filter);
    if (CurrentDisplayAdapter.D3DHWDeviceDesc.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA)
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
    else
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, SavedAppSettings.PerspectiveCorrect);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, SavedAppSettings.Dither);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, SavedAppSettings.ZBuffer);
    if (SavedAppSettings.ZBuffer && !ZBufferSurface)
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    AlphaBlendEnabler = CurrentDisplayAdapter.shadeRestricted ? D3DRENDERSTATE_STIPPLEDALPHA : D3DRENDERSTATE_ALPHABLENDENABLE;
    HWR_ResetTexSource();
    HWR_ResetColorKey();
    HWR_ResetZBuffer();
}

extern "C" void HWR_ResetTexSource(void)
{
    CurrentTexSource = 0;
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
    G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
}

void HWR_ResetColorKey()
{
    ColorKeyState = false;
    if (TexturesAlphaChannel)
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
    else
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false);
}

void HWR_ResetZBuffer()
{
    ZWriteEnableState = false;
    ZEnableState = false;
    if (SavedAppSettings.ZBuffer)
    {
        if (ZBufferSurface)
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
        else
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    }
}

void HWR_TexSource(D3DTEXTUREHANDLE tSource)
{
    if (tSource != CurrentTexSource)
    {
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, tSource);
        CurrentTexSource = tSource;
    }
}

void HWR_EnableColorKey(bool tEnable)
{
    if (tEnable != ColorKeyState)
    {
        if (TexturesAlphaChannel)
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, tEnable);
        else
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, tEnable);
        ColorKeyState = tEnable;
    }
}

void HWR_EnableZBuffer(bool tWrite, bool tCompare)
{
    if (SavedAppSettings.ZBuffer)
    {
        if (tWrite != ZWriteEnableState)
        {
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, tWrite);
            ZWriteEnableState = tWrite;
        }
        if (tCompare != ZEnableState)
        {
            if (ZBufferSurface)
                G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, tCompare ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
            else
                G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, tCompare);
            ZEnableState = tCompare;
        }
    }
}

void HWR_BeginScene()
{
    HWR_GetAllTextureHandles();
    DX_DoFlipWait();
    G_Direct3DDevice->BeginScene();
}

void HWR_EndScene()
{
    G_Direct3DDevice->EndScene();
}

extern "C" sint32 surfacenum;
extern "C" sint32 sort3d_buffer[][2];

extern "C" void HWR_DrawPolyList(void)
{
    DWORD n;
    sint32 *sort;
    sint16 *info;
    int type;
    int page;
    int count;
    D3DTLVERTEX *vtx;
    DWORD alpha;

    HWR_EnableZBuffer(false, true);
    sort = sort3d_buffer[0];
    n = surfacenum;
    while (n--) {
        info = (sint16 *)sort[0];
        type = *info++;
        switch (type) {
        case 9:
        case 10:
            page = info[0];
            count = info[1];
            vtx = *(D3DTLVERTEX **)(info + 2);
            HWR_TexSource(HWR_PageHandles[page]);
            HWR_EnableColorKey(type == 10);
            G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, vtx, count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            break;
        case 11:
            count = info[0];
            vtx = *(D3DTLVERTEX **)(info + 1);
            HWR_TexSource(0);
            HWR_EnableColorKey(false);
            G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, vtx, count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            break;
        case 12:
            count = info[0];
            vtx = *(D3DTLVERTEX **)(info + 1);
            HWR_TexSource(0);
            HWR_EnableColorKey(false);
            G_Direct3DDevice->DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, vtx, count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            break;
        case 13:
            count = info[0];
            vtx = *(D3DTLVERTEX **)(info + 1);
            HWR_TexSource(0);
            G_Direct3DDevice->GetRenderState(AlphaBlendEnabler, &alpha);
            G_Direct3DDevice->SetRenderState(AlphaBlendEnabler, TRUE);
            G_Direct3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, vtx, count, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            G_Direct3DDevice->SetRenderState(AlphaBlendEnabler, alpha);
            break;
        }
        sort += 2;
    }
}

void HWR_LoadTexturePages(int nPages, uint8 *pImage, uint8 *pPalette)
{
    HWR_FreeTexturePages();

    int nPage, nHandle;
    if (pPalette)
        PaletteIndex = CreateTexturePalette(pPalette);

    for (nPage = 0; nPage < nPages; ++nPage)
    {
        if (pPalette)
        {
            nHandle = AddTexturePage8(256, 256, pImage, PaletteIndex);
            pImage += 0x10000;
        }
        else
        {
            nHandle = AddTexturePage16(256, 256, pImage);
            pImage += 0x20000;
        }

        if (nHandle >= 0)
            LanTextureHandle[nPage] = nHandle;
        else
            LanTextureHandle[nPage] = -1;
    }

    HWR_GetAllTextureHandles();
}

void HWR_FreeTexturePages()
{
    for (int nPage = 0; nPage < ArraySize(LanTextureHandle); ++nPage)
    {
        int nHandle = LanTextureHandle[nPage];
        if (nHandle >= 0)
        {
            SafeFreeTexturePage(nHandle);
            LanTextureHandle[nPage] = -1;
        }
        HWR_PageHandles[nPage] = 0;
    }
    if (PaletteIndex >= 0)
        SafeFreePalette(PaletteIndex);
}

void HWR_GetAllTextureHandles()
{
    for (int nTPage = 0; nTPage < ArraySize(LanTextureHandle); ++nTPage)
    {
        int nHandle = LanTextureHandle[nTPage];
        if (nHandle >= 0)
            HWR_PageHandles[nTPage] = GetTexturePageHandle(nHandle);
        else
            HWR_PageHandles[nTPage] = 0;
    }
}

bool HWR_VertexBufferFull()
{
    return HWR_VertexPtr - VertexBuffer >= MAX_TLVERTICES - 512;
}

void HWR_InitVertexList()
{
    HWR_VertexPtr = VertexBuffer;
}

bool HWR_Init()
{
    Zero(VertexBuffer);
    for (int nTexture = 0; nTexture < ArraySize(LanTextureHandle); ++nTexture)
        LanTextureHandle[nTexture] = -1;

    return true;
}

#ifdef TR2_EXE
IDirect3DDevice2 *G_Direct3DDevice;
D3DTLVERTEX VertexBuffer[MAX_TLVERTICES];
D3DTEXTUREHANDLE CurrentTexSource;
DWORD HWR_PageHandles[32];
D3DTLVERTEX *HWR_VertexPtr;
bool ZEnableState;
D3DRENDERSTATETYPE AlphaBlendEnabler;
bool ColorKeyState;
bool ZWriteEnableState;
int LanTextureHandle[32];
#endif
