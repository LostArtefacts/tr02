#include "include.h"
#include <dinput.h>
#include "global.h"
#include "init_result.h"
#include "template.h"

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

extern "C" bool WinInputInit(void);

extern "C" bool Func447450(void)
{
    return WinInputInit();
}

extern "C" BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

extern "C" HWND HGameWindow;

extern "C" void DInputJoystickRelease(void)
{
}

extern "C" bool DInputJoystickCreate(void);

struct JOYSTICK : GENERIC_ADAPTER {
};

typedef LIST<JOYSTICK> JOYSTICK_LIST;
typedef LIST<JOYSTICK>::NODE JOYSTICK_NODE;

extern "C" JOYSTICK_LIST JoystickList;

extern "C" bool DInputEnumDevices(void *joystickList);

extern "C" bool WinInputInit(void)
{
    bool result;

    JoystickList.RemoveAll();
    if (!DInputCreate())
        return false;
    result = DInputEnumDevices(&JoystickList);
    DInputRelease();
    return result;
}

extern "C" bool DInputEnumDevices(void *joystickList)
{
    if (DX_TRY(DInput->EnumDevices(DIDEVTYPE_JOYSTICK, DInputEnumDevicesCallback, joystickList, DIEDFL_ATTACHEDONLY)))
        return false;
    return true;
}

extern "C" BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    JOYSTICK_LIST *joyList = (JOYSTICK_LIST *)pvRef;
    JOYSTICK &joystick = joyList->NewTail();

    joystick.SetGuid(&lpddi->guidInstance);
    strcpy(joystick.description, lpddi->tszProductName);
    strcpy(joystick.name, lpddi->tszInstanceName);
    return DIENUM_CONTINUE;
}

extern "C" JOYSTICK_NODE *GetJoystick(GUID *lpGuid)
{
    JOYSTICK_NODE *joystick;
    JOYSTICK_NODE *node;

    joystick = (JOYSTICK_NODE *)JoystickList.dwCount;
    if (!joystick)
        return joystick;
    joystick = NULL;
    if (lpGuid) {
        for (node = JoystickList.GetHead(); node; node = JoystickList.GetNext(node)) {
            if (!memcmp(lpGuid, &JoystickList[node].adapterGuid, sizeof(GUID))) {
                joystick = node;
                break;
            }
        }
    }
    if (!joystick)
        joystick = JoystickList.head;
    return joystick;
}

extern "C" void DInputKeyboardCreate(void)
{
    if (DX_TRY(DInput->CreateDevice(GUID_SysKeyboard, &IDID_SysKeyboard, NULL)))
        throw INIT_ERR_CantCreateKeyboardDevice;
    if (DX_TRY(IDID_SysKeyboard->SetCooperativeLevel(HGameWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
        throw INIT_ERR_CantSetKBCooperativeLevel;
    if (DX_TRY(IDID_SysKeyboard->SetDataFormat(&c_dfDIKeyboard)))
        throw INIT_ERR_CantSetKBDataFormat;
    if (DX_TRY(IDID_SysKeyboard->Acquire()))
        throw INIT_ERR_CantAcquireKeyboard;
}

extern "C" void DInputKeyboardRelease(void)
{
    if (IDID_SysKeyboard) {
        IDID_SysKeyboard->Unacquire();
        if (IDID_SysKeyboard) {
            IDID_SysKeyboard->Release();
            IDID_SysKeyboard = NULL;
        }
    }
}

extern "C" JOYSTICK CurrentJoystick;

extern "C" bool DInputJoystickCreate(void)
{
    if (!SavedAppSettings.PreferredJoystick)
        return true;
    CurrentJoystick = ((JOYSTICK_NODE *)SavedAppSettings.PreferredJoystick)->body;
    return true;
}

void WinInStart(void)
{
    if (!DInputCreate())
        throw INIT_ERR_CantCreateDirectInput;
    DInputKeyboardCreate();
    DInputJoystickCreate();
}

extern "C" void WinInFinish(void)
{
    DInputJoystickRelease();
    DInputKeyboardRelease();
    DInputRelease();
}

extern "C" void WinInRunControlPanel(HWND hWnd)
{
    if (DInput)
        DInput->RunControlPanel(hWnd, 0);
}

JOYSTICK_LIST JoystickList;
JOYSTICK CurrentJoystick;

#ifdef TR2_EXE
int JoyNeedCaps = 1;
JOYCAPS JoyCaps;
LPDIRECTINPUT DInput;
LPDIRECTINPUTDEVICE IDID_SysKeyboard;
#endif
