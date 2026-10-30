#include "include.h"
#include "global.h"

typedef struct {
    LPTSTR lpString;
    bool isPresented;
} STRING_FLAGGED;

typedef struct {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED driverDescription;
    STRING_FLAGGED driverName;
    DDCAPS driverCaps;
    DDCAPS helCaps;
    GUID deviceGuid;
    D3DDEVICEDESC D3DHWDeviceDesc;
} DISPLAY_ADAPTER;

extern "C" DISPLAY_ADAPTER CurrentDisplayAdapter;

extern "C" DWORD GetZBufferDepth(void)
{
    DWORD bitDepth = CurrentDisplayAdapter.D3DHWDeviceDesc.dwDeviceZBufferBitDepth;

    if (bitDepth & DDBD_16)
        return 16;
    if (bitDepth & DDBD_24)
        return 24;
    if (bitDepth & DDBD_32)
        return 32;
    return 8;
}

extern "C" bool RenderInit(void)
{
    return true;
}

extern "C" {
extern LPCTSTR ErrorStringTable[];
}

extern "C" LPCTSTR DecodeErrorMessage(DWORD errorCode)
{
    return ErrorStringTable[errorCode];
}
