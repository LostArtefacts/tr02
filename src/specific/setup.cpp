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
    uint8 pad[0x1c];
    bool JoystickEnabled;
    uint8 rest[0x10];
} SETUP_APP_SETTINGS;

extern "C" {
extern SETUP_APP_SETTINGS SavedAppSettings;
extern SETUP_APP_SETTINGS ChangedAppSettings;
void RenderFinish(bool reset);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
bool SE_GraphicsTestStart(void);
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
extern HWND GraphicsDialogHandle;
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
