#include "include.h"
#include "global.h"
#include <stdio.h>

typedef struct DISPLAY_ADAPTER_NODE DISPLAY_ADAPTER_NODE;

extern "C" {
extern LPCTSTR ErrorStringTable[];
extern DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;
}

extern "C" LPCTSTR DecodeErrorMessage(DWORD errorCode)
{
    return ErrorStringTable[errorCode];
}

#ifdef TR2_EXE
LPCTSTR ErrorStringTable[41] = {
    "OK",
    "PreferredAdapterNotFound",
    "CantCreateWindow",
    "CantCreateDirectDraw",
    "CantInitRenderer",
    "CantCreateDirectInput",
    "CantCreateKeyboardDevice",
    "CantSetKBCooperativeLevel",
    "CantSetKBDataFormat",
    "CantAcquireKeyboard",
    "CantSetDSCooperativeLevel",
    "DD_SetExclusiveMode",
    "DD_ClearExclusiveMode",
    "SetDisplayMode",
    "CreateScreenBuffers",
    "GetBackBuffer",
    "CreatePalette",
    "SetPalette",
    "CreatePrimarySurface",
    "CreateBackBuffer",
    "CreateClipper",
    "SetClipperHWnd",
    "SetClipper",
    "CreateZBuffer",
    "AttachZBuffer",
    "CreateRenderBuffer",
    "CreatePictureBuffer",
    "D3D_Create",
    "CreateDevice",
    "CreateViewport",
    "AddViewport",
    "SetViewport2",
    "SetCurrentViewport",
    "ClearRenderBuffer",
    "UpdateRenderInfo",
    "GetThirdBuffer",
    "GoFullScreen",
    "GoWindowed",
    "WrongBitDepth",
    "GetPixelFormat",
    "GetDisplayMode",
};
DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;
#endif
