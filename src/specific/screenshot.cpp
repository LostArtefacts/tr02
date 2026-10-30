#include "include.h"
#include "global.h"
#include "winmain.h"

typedef struct {
    BYTE manufacturer;
    BYTE version;
    BYTE rle;
    BYTE bpp;
    WORD xMin;
    WORD yMin;
    WORD xMax;
    WORD yMax;
    WORD hDpi;
    WORD vDpi;
    BYTE palette[48];
    BYTE reserved;
    BYTE planes;
    WORD bytesPerLine;
    WORD paletteInfo;
    WORD hScreenSize;
    WORD vScreenSize;
    BYTE filler[54];
} PCX_HEADER;

extern "C" LPDIRECTDRAWSURFACE RenderBufferSurface;
extern "C" LPDIRECTDRAWSURFACE PrimaryBufferSurface;
extern "C" int ScreenShotPcxNumber;
extern "C" char ScreenShotPcxFormat[];

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
    header->manufacturer = 10;
    header->version = 5;
    header->rle = 1;
    header->bpp = 8;
    header->planes = 1;
    header->xMax = (WORD)(width - 1);
    header->xMin = 0;
    header->yMax = (WORD)(height - 1);
    header->yMin = 0;
    header->hDpi = (WORD)width;
    header->vDpi = (WORD)height;
    header->bytesPerLine = (WORD)width;
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
