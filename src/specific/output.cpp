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
extern "C" uint8 DepthQIndex[256];
extern "C" HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc, DWORD flags);
extern "C" HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc);

inline bool DX_TRY(HRESULT hr)
{
    return FAILED(hr);
}

extern "C" void S_CopyScreenToBuffer(void)
{
    DDSURFACEDESC desc;
    uint8 *ptr;
    int x, y;

    if (SavedAppSettings.RenderMode == RM_Software)
    {
        PictureBufferSurface->Blt(NULL, RenderBufferSurface, &GameVidRect, DDBLT_WAIT, NULL);
        if (!DX_TRY(WinVidBufferLock(PictureBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
        {
            ptr = (uint8 *)desc.lpSurface;
            for (y = 0; y < 480; y++)
            {
                for (x = 0; x < 640; x++)
                {
                    *ptr = DepthQIndex[*ptr];
                    ptr++;
                }
                ptr += desc.lPitch - 640;
            }
            WinVidBufferUnlock(PictureBufferSurface, &desc);
        }
        memcpy(PicPalette, game_palette, 0x300);
    }
}

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

typedef struct {
    uint8 zsoft_flag;
    char version;
    uint8 encoding;
    uint8 bits;
    uint16 xmin;
    uint16 ymin;
    uint16 xmax;
    uint16 ymax;
    uint16 hres;
    uint16 vres;
    uint8 palette[48];
    uint8 reserved;
    uint8 planes;
    uint16 bytes_per_line;
    uint16 palette_type;
    uint8 filler[58];
} PCX_HEADER;

extern "C" int DecompPCX(uint8 *pcx, int pcx_size, uint8 *bitmap, uint8 *palette)
{
    PCX_HEADER *header;
    uint8 *in, *out, *end;
    int width, height, pixels;
    char count, c;

    header = (PCX_HEADER *)pcx;
    if (header->zsoft_flag != 10 || header->version < 5 || header->bits != 8 || header->encoding != 1 || header->planes != 1)
        return 0;
    width = header->xmax + 1 - header->xmin;
    height = header->ymax + 1 - header->ymin;
    pixels = width * height;
    if (!pixels)
        return 0;
    in = pcx + sizeof(PCX_HEADER);
    out = bitmap;
    end = out + pixels;
    while (out < end)
    {
        if ((*in & 0xc0) == 0xc0)
        {
            count = *in++;
            count &= 0x3f;
            c = *in++;
            memset(out, c, count);
            out += count;
        }
        else
            *out++ = *in++;
    }
    if (palette)
        memcpy(palette, pcx + pcx_size - 768, 768);
    return 1;
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

extern "C" void S_DrawHealthBar(int percent)
{
    int bar;
    sint32 z;
    int x;
    char colour;

    bar = percent * 100 / 100;
    bBlueEffect = false;
    z = phd_znear + 50;
    colour = inv_colours[C_BLACK];
    InsertLine(6, 7, 109, 7, z, colour);
    InsertLine(6, 8, 109, 8, z, colour);
    InsertLine(6, 9, 109, 9, z, colour);
    InsertLine(6, 10, 109, 10, z, colour);
    InsertLine(6, 11, 109, 11, z, colour);
    InsertLine(6, 12, 109, 12, z, colour);
    InsertLine(6, 13, 109, 13, z, colour);
    z = phd_znear + 40;
    colour = inv_colours[C_GREY];
    InsertLine(6, 14, 110, 14, z, colour);
    InsertLine(110, 6, 110, 14, z, colour);
    z = phd_znear + 30;
    colour = inv_colours[C_WHITE];
    InsertLine(6, 6, 110, 6, z, colour);
    InsertLine(6, 14, 6, 6, z, colour);
    if (bar)
    {
        z = phd_znear + 20;
        x = bar + 8;
        colour = inv_colours[C_ORANGE];
        InsertLine(8, 9, x, 9, z, colour);
        colour = inv_colours[C_RED];
        InsertLine(8, 8, x, 8, z, colour);
        InsertLine(8, 10, x, 10, z, colour);
        InsertLine(8, 11, x, 11, z, colour);
        InsertLine(8, 12, x, 12, z, colour);
    }
}

extern "C" sint16 DumpWidth;

extern "C" void S_DrawAirBar(int percent)
{
    int x0;
    int bar;
    sint32 z;
    char colour;

    x0 = DumpWidth - 110;
    bar = percent * 100 / 100;
    bBlueEffect = false;
    z = phd_znear + 50;
    colour = inv_colours[C_BLACK];
    InsertLine(x0 - 2, 7, x0 + 101, 7, z, colour);
    InsertLine(x0 - 2, 8, x0 + 101, 8, z, colour);
    InsertLine(x0 - 2, 9, x0 + 101, 9, z, colour);
    InsertLine(x0 - 2, 10, x0 + 101, 10, z, colour);
    InsertLine(x0 - 2, 11, x0 + 101, 11, z, colour);
    InsertLine(x0 - 2, 12, x0 + 101, 12, z, colour);
    InsertLine(x0 - 2, 13, x0 + 101, 13, z, colour);
    z = phd_znear + 40;
    colour = inv_colours[C_GREY];
    InsertLine(x0 - 2, 14, x0 + 102, 14, z, colour);
    InsertLine(x0 + 102, 6, x0 + 102, 14, z, colour);
    z = phd_znear + 30;
    colour = inv_colours[C_WHITE];
    InsertLine(x0 - 2, 6, x0 + 102, 6, z, colour);
    InsertLine(x0 - 2, 14, x0 - 2, 6, z, colour);
    if (percent > 0)
    {
        z = phd_znear + 20;
        colour = inv_colours[C_WHITE];
        InsertLine(x0, 9, x0 + bar, 9, z, colour);
        colour = inv_colours[C_BLUE];
        InsertLine(x0, 8, x0 + bar, 8, z, colour);
        InsertLine(x0, 10, x0 + bar, 10, z, colour);
        InsertLine(x0, 11, x0 + bar, 11, z, colour);
        InsertLine(x0, 12, x0 + bar, 12, z, colour);
    }
}

extern "C" bool WinVidNeedToResetBuffers;
extern "C" bool Var51bc30;
extern "C" LPDIRECTDRAWSURFACE ZBufferSurface;
extern "C" void RestoreLostBuffers(void);
extern "C" void WinVidSpinMessageLoop(bool blocking);
extern "C" void DX_DoFlipWait(void);
extern "C" void phd_InitPolyList(void);
void HWR_BeginScene(void);
void HWR_EnableZBuffer(bool zWrite, bool zEnable);

extern "C" void S_InitialisePolyList(BOOL clear_back_buffer)
{
    DWORD flags;

    if (WinVidNeedToResetBuffers)
    {
        RestoreLostBuffers();
        WinVidSpinMessageLoop(false);
        flags = 0;
        if (*((bool *)&SavedAppSettings + 0x25))
        {
            if (SavedAppSettings.RenderMode == RM_Software)
                flags = 0x10;
            flags |= 3;
            if (*((bool *)&SavedAppSettings + 0x24))
                flags |= 4;
            DX_DoFlipWait();
        }
        else
        {
            flags = SavedAppSettings.RenderMode == RM_Hardware ? 0x42 : 0x50;
        }
        ClearBuffers(flags, 0);
        clear_back_buffer = FALSE;
        WinVidNeedToResetBuffers = false;
    }
    if (SavedAppSettings.RenderMode == RM_Software)
    {
        ClearBuffers(0x110, 0);
    }
    else
    {
        flags = 0x100;
        if (clear_back_buffer || Var51bc30)
            flags = 0x102;
        if (SavedAppSettings.ZBuffer && ZBufferSurface)
            flags |= 8;
        ClearBuffers(flags, 0);
        HWR_BeginScene();
        HWR_EnableZBuffer(true, true);
    }
    phd_InitPolyList();
}

#include "../room.h"

typedef struct {
    sint32 x, y, z;
    sint16 intensity, intensity2;
    sint32 falloff, falloff2;
} LIGHT_INFO;

extern "C" sint32 number_dynamics;
extern "C" LIGHT_INFO dynamic[];

extern "C" void S_LightRoom(ROOM_INFO *r)
{
    sint32 i, j;
    sint32 x, y, z, dlx, dly, dlz, roomx, roomz;
    sint32 distance, falloff_limit, falloff, intensity, radius;
    sint16 *objptr;
    int *wibble;
    LIGHT_INFO *dl;

    if (r->lighting == 0)
    {
        if (r->flags & 0x10)
        {
            objptr = r->data;
            for (i = *(objptr++); i > 0; i--, objptr += 6)
                objptr[5] = objptr[3];
            r->flags &= ~0x10;
        }
    }
    else
    {
        wibble = wibble_light[RoomLightShades[r->lighting]];
        objptr = r->data;
        for (i = *(objptr++); i > 0; i--, objptr += 6)
            objptr[5] = objptr[3] + wibble[objptr[4] & 0x1f];
    }
    if (!number_dynamics)
        return;
    roomx = (r->y_size - 1) << WALL_SHIFT;
    roomz = (r->x_size - 1) << WALL_SHIFT;
    for (i = 0, dl = dynamic; i < number_dynamics; i++, dl++)
    {
        dlx = dl->x - r->x;
        dlz = dl->z - r->z;
        radius = 1 << dl->falloff;
        if (dlx - radius > roomx)
            continue;
        if (dlz - radius > roomz)
            continue;
        if (dlx + radius < WALL_L)
            continue;
        if (dlz + radius < WALL_L)
            continue;
        r->flags |= 0x10;
        dly = dl->y;
        falloff_limit = 1 << (dl->falloff << 1);
        falloff = (dl->falloff << 1) - dl->intensity;
        intensity = 1 << dl->intensity;
        objptr = r->data;
        for (j = *(objptr++); j > 0; j--, objptr += 6)
        {
            if (objptr[5] == 0)
                continue;
            x = objptr[0] - dlx;
            if (x < -radius || x > radius)
                continue;
            y = objptr[1] - dly;
            if (y < -radius || y > radius)
                continue;
            z = objptr[2] - dlz;
            if (z < -radius || z > radius)
                continue;
            distance = x * x + y * y + z * z;
            if (distance > falloff_limit)
                continue;
            objptr[5] -= (sint16)(intensity - (distance >> falloff));
            if (objptr[5] < 0)
                objptr[5] = 0;
        }
    }
}
