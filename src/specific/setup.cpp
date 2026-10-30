// cflags: /GX
#include <stdio.h>
#include "global.h"

extern "C" {
extern char RegistryKeyName[];
extern char RegistryKeyFormat[];
BOOL OpenRegistryKey(LPCTSTR lpSubKey);
LONG CloseRegistryKey(void);
}

extern "C" BOOL OpenGameRegistryKey(LPCTSTR key)
{
    char buffer[256];

    if (key)
    {
        sprintf(buffer, RegistryKeyFormat, RegistryKeyName, key);
        return OpenRegistryKey(buffer);
    }
    return OpenRegistryKey(RegistryKeyName);
}

extern "C" LONG CloseGameRegistryKey(void)
{
    return CloseRegistryKey();
}

typedef struct
{
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    uint8 pad[0x1a];
    bool SoundEnabled;
    bool LaraMic;
    bool JoystickEnabled;
    uint8 rest[0x10];
} SETUP_APP_SETTINGS;

extern "C" {
extern SETUP_APP_SETTINGS SavedAppSettings;
extern SETUP_APP_SETTINGS ChangedAppSettings;
void RenderFinish(bool reset);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
}

bool WinVidStart(void);
void RenderStart(bool reset);

extern "C" {
extern LPCSTR SE_PassedText;
extern LPCSTR SE_FailedText;
extern LPCSTR SE_SettingsOkText;
extern HWND GraphicsDialogHandle;
LPCSTR DecodeErrorMessage(DWORD error);
}

extern "C" bool SE_GraphicsTestStart(void)
{
    try
    {
        WinVidStart();
        RenderStart(true);
    }
    catch (DWORD error)
    {
        SetDlgItemText(GraphicsDialogHandle, 1043, SE_FailedText);
        SetDlgItemText(GraphicsDialogHandle, 1046, DecodeErrorMessage(error));
        return false;
    }
    SetDlgItemText(GraphicsDialogHandle, 1043, SE_PassedText);
    SetDlgItemText(GraphicsDialogHandle, 1046, SE_SettingsOkText);
    return true;
}

extern "C" void SE_GraphicsTestFinish(void)
{
    RenderFinish(true);
    WinVidFinish();
    WinVidHideGameWindow();
}

extern "C" bool SE_GraphicsTestExecute(void)
{
    return false;
}

extern "C" bool SE_GraphicsTest(void)
{
    SETUP_APP_SETTINGS saved;
    bool result = false;

    saved = SavedAppSettings;
    SavedAppSettings = ChangedAppSettings;
    if (SE_GraphicsTestStart())
        result = SE_GraphicsTestExecute();
    SE_GraphicsTestFinish();
    SavedAppSettings = saved;
    return result;
}

#include "ds.h"

extern "C" {
void WinSndFinish(void);
bool SE_SoundTestStart(void);
}

extern "C" void SE_SoundTestFinish(void)
{
    WinSndFinish();
}

extern "C" bool SE_SoundTestExecute(void)
{
    int handle;

    handle = DS_StartSample(0, -1000, 0x10000, 0, 0);
    if (handle >= 0)
    {
        while (DS_IsChannelPlaying(handle))
            ;
    }
    return false;
}

extern "C" bool SE_SoundTest(void)
{
    SETUP_APP_SETTINGS saved;
    bool result = false;

    saved = SavedAppSettings;
    SavedAppSettings = ChangedAppSettings;
    if (SE_SoundTestStart())
        result = SE_SoundTestExecute();
    SE_SoundTestFinish();
    SavedAppSettings = saved;
    return result;
}

extern "C" {
extern WNDPROC SE_OldPropSheetWndProc;
extern HWND SE_PropSheetWndHandle;
extern HWND GraphicsDialogHandle;
extern HWND SoundDialogHandle;
}

extern "C" LRESULT CALLBACK SE_NewPropSheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern "C" int CALLBACK SE_PropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == 2)
    {
        GraphicsDialogHandle = NULL;
        return 0;
    }
    if (uMsg == 1)
    {
        SE_PropSheetWndHandle = hwndDlg;
        SE_OldPropSheetWndProc = (WNDPROC)SetWindowLong(hwndDlg, GWL_WNDPROC, (LONG)SE_NewPropSheetWndProc);
    }
    return 0;
}

extern "C" LRESULT CALLBACK SE_NewPropSheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        SetWindowLong(hWnd, GWL_WNDPROC, (LONG)SE_OldPropSheetWndProc);
        break;
    case WM_DISPLAYCHANGE:
        if (GraphicsDialogHandle)
            PostMessage(GraphicsDialogHandle, uMsg, wParam, lParam);
        break;
    }
    return CallWindowProc((FARPROC)SE_OldPropSheetWndProc, hWnd, uMsg, wParam, lParam);
}

extern "C" void SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg);

extern "C" void SE_GraphicsAdapterSet(HWND hwndDlg, void *adapter)
{
    ChangedAppSettings.PreferredDisplayAdapter = adapter;
    SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
}

extern "C" void SE_SoundAdapterSet(HWND hwndDlg, void *adapter)
{
    ChangedAppSettings.PreferredSoundAdapter = adapter;
}

extern "C" void SE_ControlsJoystickSet(HWND hwndDlg, void *joystick)
{
    ChangedAppSettings.PreferredJoystick = joystick;
}

extern "C" void SE_OptionsStrCat(LPSTR *dst, bool isEnabled, bool *isNext, LPCSTR src)
{
    if (!isEnabled)
        return;
    if (*isNext)
    {
        *(*dst)++ = ',';
        *(*dst)++ = ' ';
    }
    while (*src)
        *(*dst)++ = *src++;
    *isNext = true;
}

extern "C" void SE_AdvancedDlgUpdate(HWND hwndDlg);

extern "C" void SE_AdvancedDlgInit(HWND hwndDlg)
{
    SE_AdvancedDlgUpdate(hwndDlg);
}

extern "C" char SetupDialogName[];

extern "C" HWND SE_FindSetupDialog(void)
{
    return FindWindow(MAKEINTATOM(0x8002), SetupDialogName);
}

void UT_CenterWindow(HWND hWnd);

extern "C" {
extern bool IsSetupDialogCentered;
extern HWND OptionsDialogHandle;
extern HWND OptionsImageHandle;
void Func457470(HWND hWnd, UINT uMsg, WPARAM wParam);
void SE_OptionsDlgUpdate(HWND hwndDlg);
}

extern "C" BOOL CALLBACK SE_OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        OptionsDialogHandle = hwndDlg;
        if (!IsSetupDialogCentered)
        {
            UT_CenterWindow(GetParent(hwndDlg));
            IsSetupDialogCentered = true;
        }
        OptionsImageHandle = GetDlgItem(hwndDlg, 1073);
        return TRUE;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == (UINT)-200)
            SE_OptionsDlgUpdate(hwndDlg);
        break;
    }
    Func457470(OptionsImageHandle, uMsg, wParam);
    return FALSE;
}

extern "C" {
extern HWND ControlsDialogHandle;
extern HWND ControlsImageHandle;
void SE_ControlsDlgInit(HWND hwndDlg);
void SE_ControlsDlgUpdate(HWND hwndDlg);
void DInputCreate(void);
void DInputRelease(void);
void WinInRunControlPanel(HWND hWnd);
}

extern "C" BOOL CALLBACK SE_ControlsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ControlsDialogHandle = hwndDlg;
        ControlsImageHandle = GetDlgItem(hwndDlg, 1075);
        SE_ControlsDlgInit(hwndDlg);
        return TRUE;
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            result = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            result = SendMessage((HWND)lParam, CB_GETITEMDATA, result, 0);
            if (LOWORD(wParam) == 1055)
            {
                SE_ControlsJoystickSet(hwndDlg, (void *)result);
                SE_ControlsDlgUpdate(hwndDlg);
            }
        }
        else if (HIWORD(wParam) == BN_CLICKED)
        {
            result = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
            switch (LOWORD(wParam))
            {
            case 1099:
                ChangedAppSettings.JoystickEnabled = result == BST_CHECKED;
                break;
            case 1100:
                DInputCreate();
                WinInRunControlPanel(hwndDlg);
                DInputRelease();
                break;
            }
        }
        break;
    }
    Func457470(ControlsImageHandle, uMsg, wParam);
    return FALSE;
}

typedef struct
{
    void *head;
    void *tail;
    int count;
} SETUP_SOUND_LIST;

extern "C" SETUP_SOUND_LIST SoundAdapterList;

inline void SE_EnableItem(HWND hwndDlg, int id, bool enable)
{
    EnableWindow(GetDlgItem(hwndDlg, id), enable);
}

inline void SE_CheckItem(HWND hwndDlg, int id, bool enable, bool check)
{
    HWND item = GetDlgItem(hwndDlg, id);
    EnableWindow(item, enable);
    SendMessage(item, BM_SETCHECK, check, 0);
}

extern "C" void SE_SoundDlgUpdate(HWND hwndDlg)
{
    int count = SoundAdapterList.count;
    bool isAvailable = count > 0;

    SE_EnableItem(hwndDlg, 1097, isAvailable);
    SE_EnableItem(hwndDlg, 1098, isAvailable);
    SE_CheckItem(hwndDlg, 1057, isAvailable, ChangedAppSettings.SoundEnabled && isAvailable);
    SE_CheckItem(hwndDlg, 1086, isAvailable, !ChangedAppSettings.LaraMic);
    SE_CheckItem(hwndDlg, 1087, isAvailable, ChangedAppSettings.LaraMic);
    SE_EnableItem(hwndDlg, 1089, isAvailable);
}

extern "C" {
extern HWND SoundImageHandle;
void SE_SoundDlgInit(HWND hwndDlg);
}

extern "C" BOOL CALLBACK SE_SoundDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SoundDialogHandle = hwndDlg;
        SoundImageHandle = GetDlgItem(hwndDlg, 1074);
        SE_SoundDlgInit(hwndDlg);
        return TRUE;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            result = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
            switch (LOWORD(wParam))
            {
            case 1057:
                ChangedAppSettings.SoundEnabled = result == BST_CHECKED;
                break;
            case 1086:
                ChangedAppSettings.LaraMic = false;
                break;
            case 1087:
                ChangedAppSettings.LaraMic = true;
                break;
            case 1089:
                if (SE_SoundTest())
                    PostMessage(SE_PropSheetWndHandle, WM_USER + 113, 5, 0);
                SendMessage((HWND)lParam, BM_SETCHECK, 0, 0);
                break;
            }
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            result = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            result = SendMessage((HWND)lParam, CB_GETITEMDATA, result, 0);
            if (LOWORD(wParam) == 1052)
            {
                SE_SoundAdapterSet(hwndDlg, (void *)result);
                SE_SoundDlgUpdate(hwndDlg);
            }
        }
        break;
    }
    Func457470(SoundImageHandle, uMsg, wParam);
    return FALSE;
}

typedef struct
{
    void *head;
    void *tail;
    int count;
} SETUP_JOYSTICK_LIST;

extern "C" SETUP_JOYSTICK_LIST JoystickList;

extern "C" void SE_ControlsDlgUpdate(HWND hwndDlg)
{
    int count = JoystickList.count;
    bool isAvailable = count > 0;

    SE_CheckItem(hwndDlg, 1099, isAvailable, ChangedAppSettings.JoystickEnabled && isAvailable);
}
