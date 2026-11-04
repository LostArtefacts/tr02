#include "include.h"
#include "global.h"
#include "winmain.h"
#include <stdio.h>
#include "stypes.h"

extern "C" LPDIRECTDRAWSURFACE RenderBufferSurface;
extern "C" LPDIRECTDRAWSURFACE PrimaryBufferSurface;
extern "C" int ScreenShotPcxNumber;
extern "C" char ScreenShotPcxFormat[];

extern "C" int ScreenShotTgaNumber;
extern "C" char ScreenShotTgaFormat[];
extern "C" char TgaFileMode[];
extern "C" BYTE TgaHeader[];
extern "C" BYTE *malloc_ptr;

extern "C" HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc, DWORD flags);
extern "C" HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE surface, DDSURFACEDESC *desc);
extern "C" DWORD CompPCX(BYTE *bitmap, int width, int height, BYTE *palette, BYTE **pcxData);
extern "C" DWORD EncodePutPCX(BYTE value, BYTE num, BYTE *buffer);
extern "C" DWORD EncodeLinePCX(BYTE *src, int width, BYTE *dst);

extern "C" void ScreenShotPCX(void)
{
    LPDIRECTDRAWSURFACE surface;
    DDSURFACEDESC desc;
    HRESULT rc;
    BYTE *pcxData;
    DWORD size;
    DWORD written;
    HANDLE file;
    char fileName[20];

    surface = RenderBufferSurface;
    if (SavedAppSettings.RenderMode != RM_Software)
        surface = PrimaryBufferSurface;
    desc.dwSize = sizeof(DDSURFACEDESC);
    do {
        rc = surface->Lock(NULL, &desc, 0, NULL);
    } while (rc == DDERR_WASSTILLDRAWING);
    if (rc == DDERR_SURFACELOST) {
        surface->Restore();
        return;
    }
    if (FAILED(rc))
        return;
    size = CompPCX((BYTE *)desc.lpSurface, desc.dwWidth, desc.dwHeight, game_palette, &pcxData);
    surface->Unlock(&desc);
    if (!size)
        return;
    if (++ScreenShotPcxNumber > 9999)
        ScreenShotPcxNumber = 1;
    wsprintf(fileName, ScreenShotPcxFormat, ScreenShotPcxNumber);
    file = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(file, pcxData, size, &written, NULL);
    CloseHandle(file);
    GlobalFree(pcxData);
}

extern "C" DWORD CompPCX(BYTE *bitmap, int width, int height, BYTE *palette, BYTE **pcxData)
{
    int i;
    PCX_HEADER *header;
    BYTE *pic;

    header = (PCX_HEADER *)GlobalAlloc(GMEM_FIXED, height * width * 2 + sizeof(PCX_HEADER) + 768);
    if (!header)
        return 0;
    *pcxData = (BYTE *)header;
    header->zsoft_flag = 10;
    header->version = 5;
    header->encoding = 1;
    header->bits = 8;
    header->planes = 1;
    header->xmax = (WORD)(width - 1);
    header->xmin = 0;
    header->ymax = (WORD)(height - 1);
    header->ymin = 0;
    header->h_res = (WORD)width;
    header->v_res = (WORD)height;
    header->bytes_per_line = (WORD)width;
    pic = (BYTE *)header + sizeof(PCX_HEADER);
    for (i = 0; i < height; i++) {
        pic += EncodeLinePCX(bitmap, width, pic);
        bitmap += width;
    }
    *pic++ = 12;
    for (i = 0; i < 768; i++)
        *pic++ = *palette++;
    return pic - *pcxData;
}

extern "C" DWORD EncodeLinePCX(BYTE *src, int width, BYTE *dst)
{
    BYTE current;
    DWORD add;
    DWORD total = 0;
    BYTE run = 1;
    BYTE last = *src;
    int i;

    for (i = 1; i < width; i++) {
        current = *++src;
        if (current == last) {
            run++;
            if (run == 63) {
                add = EncodePutPCX(last, run, dst);
                if (!add)
                    return 0;
                total += add;
                dst += add;
                run = 0;
            }
        } else {
            if (run) {
                add = EncodePutPCX(last, run, dst);
                if (!add)
                    return 0;
                total += add;
                dst += add;
            }
            last = current;
            run = 1;
        }
    }
    if (run) {
        add = EncodePutPCX(last, run, dst);
        if (!add)
            return 0;
        total += add;
        dst += add;
    }
    return total;
}

extern "C" DWORD EncodePutPCX(BYTE value, BYTE num, BYTE *buffer)
{
    if (!num || (char)num > 63)
        return 0;
    if (num == 1 && (value & 0xc0) != 0xc0)
    {
        buffer[0] = value;
        return 1;
    }
    buffer[0] = num | 0xc0;
    buffer[1] = value;
    return 2;
}

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" void ScreenShot(LPDIRECTDRAWSURFACE screen)
{
    DDSURFACEDESC desc;
    char fileName[16];
    FILE *fp;
    BYTE *surface;
    short *src;
    short *line;
    WORD *dst;
    DWORD x;
    DWORD y;
    DWORD pixel;

    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    if (DX_TRY(screen->GetSurfaceDesc(&desc)))
        return;
    if (desc.ddpfPixelFormat.dwRGBBitCount == 8) {
        ScreenShotPCX();
        return;
    }
    if (desc.ddpfPixelFormat.dwRGBBitCount != 16)
        return;
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    if ((bool)FAILED(WinVidBufferLock(screen, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
        return;
    surface = (BYTE *)desc.lpSurface;
    sprintf(fileName, ScreenShotTgaFormat, ScreenShotTgaNumber++);
    fp = fopen(fileName, TgaFileMode);
    if (fp) {
        *(WORD *)&TgaHeader[12] = (WORD)desc.dwWidth;
        *(WORD *)&TgaHeader[14] = (WORD)desc.dwHeight;
        fwrite(TgaHeader, 18, 1, fp);
        dst = (WORD *)malloc_ptr;
        line = (short *)surface + desc.lPitch / 2 * desc.dwHeight;
        for (y = 0; y < desc.dwHeight; y++) {
            src = line;
            for (x = 0; x < desc.dwWidth; x++) {
                pixel = *src;
                if (desc.ddpfPixelFormat.dwRBitMask == 0xf800)
                    *dst = ((((pixel >> 11) & 0x1f) << 5) + ((pixel >> 6) & 0x1f) << 5) + (pixel & 0x1f);
                else
                    *dst = (WORD)pixel;
                dst++;
                src++;
            }
            line -= desc.lPitch / 2;
        }
        fwrite(malloc_ptr, desc.dwWidth * desc.dwHeight * 2, 1, fp);
        fclose(fp);
        if (++fileName[7] > '9') {
            fileName[7] = '0';
            fileName[6]++;
        }
    }
    WinVidBufferUnlock(screen, &desc);
}

#ifdef TR2_EXE
BYTE TgaHeader[18] = {
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 1, 0, 1, 16, 0,
};
char ScreenShotPcxFormat[] = "tomb%04d.pcx";
char TgaFileMode[] = "wb";
char ScreenShotTgaFormat[] = "tomb%04d.tga";
int ScreenShotPcxNumber;
int ScreenShotTgaNumber;
#endif
