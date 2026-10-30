#include "include.h"
#include "global.h"

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

extern "C" DWORD EncodePutPCX(BYTE value, BYTE num, BYTE *buffer);
extern "C" DWORD EncodeLinePCX(BYTE *src, int width, BYTE *dst);

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
