#include "global.h"
#include "output.h"
#include "../inventry.h"
#include "winmain.h"
#include "display.h"
#include "time.h"
#include "../gameflow.h"

extern "C" void Func444560(void);
extern "C" void HWR_InitState(void);
extern "C" void ClearBuffers(DWORD dwClrFlags, DWORD dwColour);

#define ANIMATED_TEXTURE 10

int S_DumpScreen(void)
{
    int nframes;

    nframes = Sync();
    while (nframes < 2)
    {
        while (!Sync())
            ;
        nframes++;
    }

    ScreenPartialDump();

    return nframes;
}

void S_ClearScreen(void)
{
    ScreenClear();
}

void S_InitialiseScreen(int type)
{
    if (type >= 0)
    {
        if (type != IL_TITLE)
            TempVideoRemove();
        FadeToPal(30, game_palette);
    }
    else
        FadeToPal(0, game_palette);

    if (SavedAppSettings.RenderMode != RM_Software)
        HWR_InitState();
}

extern "C" void phd_SortPolyList(void);
extern "C" void HWR_DrawPolyList(void);
extern "C" void Func455940(void);
void HWR_EndScene();

extern "C" void S_OutputPolyList(void)
{
    if (SavedAppSettings.RenderMode == RM_Software)
    {
        phd_SortPolyList();
        Func455940();
    }
    else
    {
        if (!SavedAppSettings.ZBuffer || !*((bool *)&SavedAppSettings + 0x2a))
            phd_SortPolyList();
        HWR_DrawPolyList();
        HWR_EndScene();
    }
}

void S_InsertBackPolygon(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int colour)
{
    min_x += phd_winxmin;
    max_x += phd_winxmin;
    min_y += phd_winymin;
    max_y += phd_winymin;
    InsertFlatRect(min_x, min_y, max_x, max_y, phd_zfar + 1, inv_colours[C_BLACK]);
}

void AnimateTextures(int nframes)
{
    int i, j;
    sint16 *ptr;
    PHDTEXTURESTRUCT temp;
    static int frame_comp = 0;

    frame_comp += nframes;
    while (frame_comp > ANIMATED_TEXTURE)
    {
        ptr = aranges;
        for (i = (int)*(ptr++); i > 0; i--, ptr++)
        {
            j = (int)*(ptr++);
            temp = phdtextinfo[*(ptr)];
            for (; j > 0; j--, ptr++)
                phdtextinfo[*(ptr)] = phdtextinfo[*(ptr + 1)];
            phdtextinfo[*(ptr)] = temp;
        }
        frame_comp -= ANIMATED_TEXTURE;
    }
}

void S_SetupBelowWater(int underwater)
{
    if (underwater ^ wet)
    {
        if (underwater)
            FadeToPal(1, water_palette);
        else
            FadeToPal(1, game_palette);
        wet = underwater;
    }
    water_effect = !underwater;
    shade_effect = 1;
    bBlueEffect = true;
}

void S_SetupAboveWater(int underwater)
{
    water_effect = underwater;
    shade_effect = 0;
    bBlueEffect = bool(underwater);
}

extern "C" char *GetFullPath(char *path);
extern "C" void init_game_malloc(void);
extern "C" void *game_malloc(int size, int type);
extern "C" void game_free(int size, int type);
extern "C" int DecompPCX(uint8 *pcx, int pcx_size, uint8 *bitmap, uint8 *palette);
struct IDirectDrawSurface;
extern "C" void WinVidCopyBitmapToBuffer(IDirectDrawSurface *surface, uint8 *bitmap);
extern "C" void BGND_Make640x480(uint8 *bitmap, uint8 *palette);
extern "C" void CopyBitmapPalette(uint8 *src_palette, uint8 *bitmap, int size, uint8 *dst_palette);
extern "C" IDirectDrawSurface *PictureBufferSurface;
extern "C" uint8 PicPalette[];

extern "C" void S_DisplayPicture(char *file_name, int is_title)
{
    char name[100];
    HANDLE file;
    DWORD size, read;
    uint8 *pcx, *bitmap;

    lstrcpy(name, file_name);
    file = CreateFile(GetFullPath(name), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;
    if (!is_title)
        init_game_malloc();
    size = GetFileSize(file, NULL);
    pcx = (uint8 *)game_malloc(size, 40);
    ReadFile(file, pcx, 640 * 480, &read, NULL);
    CloseHandle(file);
    bitmap = (uint8 *)game_malloc(640 * 480, 40);
    DecompPCX(pcx, size, bitmap, PicPalette);
    if (SavedAppSettings.RenderMode == RM_Software)
        WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmap);
    else
        BGND_Make640x480(bitmap, PicPalette);
    if (!is_title)
        CopyBitmapPalette(PicPalette, bitmap, 640 * 480, game_palette);
    game_free(size + 640 * 480, 40);
}

#define WIBBLE_SIZE 32
#define SUNSET_TIME (20 * 60 * 60)

extern "C" int WibbleOffset;
extern "C" int RoomLightShades[4];
extern "C" sint16 GF_SunsetEnabled;
extern "C" int SunsetTimer;
extern "C" sint32 GetRandomDraw(void);

extern "C" void S_AnimateTextures(int nframes)
{
    WibbleOffset = (WibbleOffset + nframes / 2) & (WIBBLE_SIZE - 1);
    RoomLightShades[1] = GetRandomDraw() & (WIBBLE_SIZE - 1);
    RoomLightShades[2] = (phd_sin(WibbleOffset * 65536 / WIBBLE_SIZE) + (1 << W2V_SHIFT)) * (WIBBLE_SIZE - 1) >> (W2V_SHIFT + 1);
    if (GF_SunsetEnabled)
    {
        SunsetTimer += nframes;
        if (SunsetTimer < SUNSET_TIME)
            RoomLightShades[3] = SunsetTimer * (WIBBLE_SIZE - 1) / SUNSET_TIME;
        else
            RoomLightShades[3] = WIBBLE_SIZE - 1;
    }
    AnimateTextures(nframes);
}

void S_DontDisplayPicture(void)
{
    if (SavedAppSettings.RenderMode == RM_Hardware)
        Func444560();
}

void ScreenDump(void)
{
    DX_UpdateFrame(true, 0);
}

void ScreenPartialDump(void)
{
    DX_UpdateFrame(true, &GrcDumpWindow);
}

void ScreenClear(bool tWindow)
{
    DWORD dwFlags = SavedAppSettings.RenderMode == RM_Software ? 0x10 : 2;
    if (tWindow)
        dwFlags |= 0x100;
    ClearBuffers(dwFlags, 0);
}

#include "include.h"

extern "C" uint8 PicPalette[];
extern "C" LPDIRECTDRAWSURFACE RenderBufferSurface;
extern "C" LPDIRECTDRAWSURFACE PictureBufferSurface;
extern "C" RECT GameVidRect;
extern "C" void S_SyncPictureBufferPalette(void);
extern "C" void Func444350(void);

extern "C" void S_CopyBufferToScreen(void)
{
    if (SavedAppSettings.RenderMode == RM_Software)
    {
        if (memcmp(game_palette, PicPalette, 0x300))
            S_SyncPictureBufferPalette();
        RenderBufferSurface->Blt(&GameVidRect, PictureBufferSurface, NULL, DDBLT_WAIT, NULL);
    }
    else
        Func444350();
}

extern "C" sint32 LsAdder;

extern "C" void S_CalculateStaticLight(sint16 adder)
{
    sint32 depth;

    LsAdder = adder - 0x1000;
    depth = phd_mxptr[M23] >> W2V_SHIFT;
    if (depth > 0x3000)
        LsAdder += depth - 0x3000;
}

inline bool DX_TRY(HRESULT hr)
{
    return FAILED(hr);
}

extern "C" HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc, DWORD flags);
extern "C" HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc);
extern "C" void SyncSurfacePalettes(void *srcData, int width, int height, int srcPitch, uint8 *srcPalette, void *dstData, int dstPitch, uint8 *dstPalette, bool preserveBlack);

extern "C" void S_SyncPictureBufferPalette(void)
{
    DDSURFACEDESC desc;

    if (PictureBufferSurface == NULL || DX_TRY(WinVidBufferLock(PictureBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
        return;
    SyncSurfacePalettes(desc.lpSurface, 640, 480, desc.lPitch, PicPalette, desc.lpSurface, desc.lPitch, game_palette, true);
    WinVidBufferUnlock(PictureBufferSurface, &desc);
    memcpy(PicPalette, game_palette, 768);
}
