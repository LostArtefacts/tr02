#include "include.h"
#include <dinput.h>
#include "global.h"

extern "C" {
extern HINSTANCE GhInstance;
extern LPDIRECTINPUT DInput;
extern LPDIRECTINPUTDEVICE IDID_SysKeyboard;
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

extern "C" void WinInReadKeyboard(LPVOID lpInputData)
{
    HRESULT hr = IDID_SysKeyboard->GetDeviceState(256, lpInputData);

    if (FAILED(hr) && hr == DIERR_INPUTLOST) {
        if (DX_TRY(IDID_SysKeyboard->Acquire()) || DX_TRY(IDID_SysKeyboard->GetDeviceState(256, lpInputData)))
            memset(lpInputData, 0, 256);
    }
}
