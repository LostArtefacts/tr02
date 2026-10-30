#include "include.h"
#include <dinput.h>
#include "global.h"

extern "C" {
extern HINSTANCE GhInstance;
extern LPDIRECTINPUT DInput;
}

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" bool DInputCreate(void)
{
    if (DX_TRY(DirectInputCreate(GhInstance, DIRECTINPUT_VERSION, &DInput, NULL)))
        return false;
    return true;
}

extern "C" void DInputRelease(void)
{
    if (DInput) {
        DInput->Release();
        DInput = NULL;
    }
}
