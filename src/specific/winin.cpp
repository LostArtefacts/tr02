#include "include.h"
#include <dinput.h>
#include "global.h"

typedef struct {
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    void *VideoMode;
    int RenderMode;
    int WindowWidth;
    int WindowHeight;
    int AspectMode;
    bool PerspectiveCorrect;
    bool Dither;
    bool ZBuffer;
    bool BilinearFiltering;
    bool TripleBuffering;
    bool FullScreen;
    bool SoundEnabled;
    bool LaraMic;
    bool JoystickEnabled;
} APP_SETTINGS;

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

extern "C" {
extern APP_SETTINGS SavedAppSettings;
extern int JoyNeedCaps;
extern JOYCAPS JoyCaps;
}

extern "C" DWORD WinInReadJoystick(int *x, int *y)
{
    JOYINFOEX joy;

    if (!SavedAppSettings.JoystickEnabled) {
        *y = 0;
        *x = 0;
        return 0;
    }

    joy.dwSize = sizeof(JOYINFOEX);
    joy.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS;
    if (joyGetPosEx(0, &joy) != JOYERR_NOERROR) {
        JoyNeedCaps = 1;
        *y = 0;
        *x = 0;
        return 0;
    }

    if (JoyNeedCaps) {
        if (joyGetDevCaps(0, &JoyCaps, sizeof(JOYCAPS)) == JOYERR_NOERROR) {
            JoyNeedCaps = 0;
        } else {
            *y = 0;
            *x = 0;
            return 0;
        }
    }

    *x = 32 * joy.dwXpos / (JoyCaps.wXmax - JoyCaps.wXmin) - 16;
    *y = 32 * joy.dwYpos / (JoyCaps.wYmax - JoyCaps.wYmin) - 16;
    return joy.dwButtons;
}
