// cflags: /GX
#include <stdio.h>
#include "global.h"
#include <commctrl.h>

extern "C" {
extern char RegistryKeyName[];
extern char RegistryKeyFormat[];
bool OpenRegistryKey(LPCTSTR lpSubKey);
LONG CloseRegistryKey(void);
}

extern "C" bool OpenGameRegistryKey(LPCTSTR key)
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

#pragma pack(push, 1)
typedef struct
{
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
    bool Disable16BitTextures;
    bool DontSortPrimitives;
    bool FlipBroken;
    bool DisableFMV;
    int TexelAdjustMode;
    int NearestAdjustment;
    int LinearAdjustment;
} SETUP_APP_SETTINGS;
#pragma pack(pop)

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

struct DISPLAY_MODE
{
    int width;
    int height;
    int bpp;
    int vga;
    bool CompareVideoModes(DISPLAY_MODE *mode);
};

struct SETUP_MODE_NODE
{
    SETUP_MODE_NODE *next;
    SETUP_MODE_NODE *previous;
    DISPLAY_MODE body;
};

typedef struct
{
    SETUP_MODE_NODE *head;
    SETUP_MODE_NODE *tail;
    DWORD count;
} SETUP_MODE_LIST;

typedef struct
{
    void *next;
    void *previous;
    GUID *lpGuid;
    GUID guid;
} SETUP_LIST_NODE;

extern "C" {
extern void *PrimaryDisplayAdapter;
extern void *PrimarySoundAdapter;
LPCTSTR GuidBinaryToString(GUID *guid);
void SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value);
void SetRegistryBoolValue(LPCTSTR lpValueName, bool value);
void SetRegistryStringValue(LPCTSTR lpValueName, LPCTSTR value, int length);
void DeleteRegistryValue(LPCTSTR lpValueName);
}

extern "C" bool Func4524e0(SETUP_APP_SETTINGS *settings)
{
    SETUP_MODE_NODE *mode;

    if (!OpenGameRegistryKey("System"))
        return false;
    if (settings->PreferredDisplayAdapter && settings->PreferredDisplayAdapter != PrimaryDisplayAdapter)
        SetRegistryStringValue("PreferredDisplayAdapterGUID", GuidBinaryToString(&((SETUP_LIST_NODE *)settings->PreferredDisplayAdapter)->guid), 37);
    else
        DeleteRegistryValue("PreferredDisplayAdapterGUID");
    if (settings->PreferredSoundAdapter && settings->PreferredSoundAdapter != PrimarySoundAdapter)
        SetRegistryStringValue("PreferredSoundAdapterGUID", GuidBinaryToString(&((SETUP_LIST_NODE *)settings->PreferredSoundAdapter)->guid), 37);
    else
        DeleteRegistryValue("PreferredSoundAdapterGUID");
    if (settings->PreferredJoystick)
        SetRegistryStringValue("PreferredJoystickGUID", GuidBinaryToString(&((SETUP_LIST_NODE *)settings->PreferredJoystick)->guid), 37);
    else
        DeleteRegistryValue("PreferredJoystickGUID");
    SetRegistryDwordValue("RenderMode", settings->RenderMode);
    SetRegistryBoolValue("PerspectiveCorrect", settings->PerspectiveCorrect);
    SetRegistryBoolValue("Dither", settings->Dither);
    SetRegistryBoolValue("ZBuffer", settings->ZBuffer);
    SetRegistryBoolValue("BilinearFiltering", settings->BilinearFiltering);
    SetRegistryBoolValue("TripleBuffering", settings->TripleBuffering);
    SetRegistryBoolValue("FullScreen", settings->FullScreen);
    SetRegistryDwordValue("WindowWidth", settings->WindowWidth);
    SetRegistryDwordValue("WindowHeight", settings->WindowHeight);
    SetRegistryDwordValue("AspectMode", settings->AspectMode);
    SetRegistryDwordValue("NearestAdjustment", settings->NearestAdjustment);
    SetRegistryDwordValue("LinearAdjustment", settings->LinearAdjustment);
    SetRegistryBoolValue("DisableFMV", settings->DisableFMV);
    if (settings->PreferredDisplayAdapter && (mode = (SETUP_MODE_NODE *)settings->VideoMode))
    {
        SetRegistryDwordValue("FullScreenWidth", mode->body.width);
        SetRegistryDwordValue("FullScreenHeight", mode->body.height);
        SetRegistryDwordValue("FullScreenBPP", mode->body.bpp);
    }
    else
    {
        DeleteRegistryValue("FullScreenWidth");
        DeleteRegistryValue("FullScreenHeight");
        DeleteRegistryValue("FullScreenBPP");
    }
    SetRegistryBoolValue("SoundEnabled", settings->SoundEnabled);
    SetRegistryBoolValue("LaraMic", settings->LaraMic);
    SetRegistryBoolValue("JoystickEnabled", settings->JoystickEnabled);
    SetRegistryBoolValue("Disable16BitTextures", settings->Disable16BitTextures);
    SetRegistryBoolValue("DontSortPrimitives", settings->DontSortPrimitives);
    SetRegistryDwordValue("TexelAdjustMode", settings->TexelAdjustMode);
    CloseGameRegistryKey();
    return true;
}

#pragma pack(push, 1)
struct SETUP_ADAPTER_NODE
{
    SETUP_ADAPTER_NODE *next;
    SETUP_ADAPTER_NODE *previous;
    GUID *lpAdapterGuid;
    GUID adapterGuid;
    LPSTR driverDescription;
    uint8 pad0[0x3ba];
    SETUP_MODE_LIST hwDispModeList;
    SETUP_MODE_LIST swDispModeList;
    uint8 pad1[0x24];
    bool hwRenderSupported;
    uint8 pad2[4];
    bool PerspectiveCorrectSupported;
    bool DitherSupported;
    bool ZBufferSupported;
    bool LinearFilterSupported;
};
#pragma pack(pop)

typedef struct
{
    SETUP_ADAPTER_NODE *head;
    SETUP_ADAPTER_NODE *tail;
    DWORD count;
} SETUP_ADAPTER_LIST;

extern "C" SETUP_ADAPTER_LIST DisplayAdapterList;

extern "C" {
bool GetRegistryGuidValue(LPCTSTR lpValueName, GUID *value, GUID *defaultValue);
bool GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue);
bool GetRegistryBoolValue(LPCTSTR lpValueName, bool *pValue, bool defaultValue);
bool IsNewRegistryKeyCreated(void);
void *WinVidGetDisplayAdapter(GUID *lpGuid);
void *GetSoundAdapter(GUID *lpGuid);
void *GetJoystick(GUID *lpGuid);
}

#pragma pack(push, 1)
typedef struct
{
    GUID *lpAdapterGuid;
    uint8 pad[0x3ce];
    SETUP_MODE_LIST hwDispModeList;
    SETUP_MODE_LIST swDispModeList;
} SETUP_ADAPTER_BODY;

typedef struct
{
    void *next;
    void *previous;
    SETUP_ADAPTER_BODY body;
} SETUP_ADAPTER_ITEM;
#pragma pack(pop)

extern "C" int Func452760(SETUP_APP_SETTINGS *settings)
{
    GUID guid;
    DISPLAY_MODE target;
    SETUP_MODE_LIST *modeList;
    SETUP_MODE_NODE *mode;
    SETUP_ADAPTER_BODY *body;

    if (!OpenGameRegistryKey("System"))
        return 0;
    settings->PreferredDisplayAdapter = WinVidGetDisplayAdapter(GetRegistryGuidValue("PreferredDisplayAdapterGUID", &guid, NULL) ? &guid : NULL);
    settings->PreferredSoundAdapter = GetSoundAdapter(GetRegistryGuidValue("PreferredSoundAdapterGUID", &guid, NULL) ? &guid : NULL);
    settings->PreferredJoystick = GetJoystick(GetRegistryGuidValue("PreferredJoystickGUID", &guid, NULL) ? &guid : NULL);
    GetRegistryDwordValue("RenderMode", (DWORD *)&settings->RenderMode, 2);
    GetRegistryBoolValue("Dither", &settings->Dither, false);
    GetRegistryBoolValue("ZBuffer", &settings->ZBuffer, true);
    GetRegistryBoolValue("BilinearFiltering", &settings->BilinearFiltering, true);
    GetRegistryBoolValue("TripleBuffering", &settings->TripleBuffering, false);
    GetRegistryBoolValue("FullScreen", &settings->FullScreen, true);
    GetRegistryDwordValue("WindowWidth", (DWORD *)&settings->WindowWidth, 512);
    GetRegistryDwordValue("WindowHeight", (DWORD *)&settings->WindowHeight, 384);
    GetRegistryDwordValue("AspectMode", (DWORD *)&settings->AspectMode, 0);
    GetRegistryDwordValue("FullScreenWidth", (DWORD *)&target.width, 640);
    GetRegistryDwordValue("FullScreenHeight", (DWORD *)&target.height, 480);
    GetRegistryDwordValue("FullScreenBPP", (DWORD *)&target.bpp, 16);
    GetRegistryBoolValue("SoundEnabled", &settings->SoundEnabled, true);
    GetRegistryBoolValue("LaraMic", &settings->LaraMic, false);
    GetRegistryBoolValue("JoystickEnabled", &settings->JoystickEnabled, true);
    GetRegistryBoolValue("Disable16BitTextures", &settings->Disable16BitTextures, false);
    GetRegistryBoolValue("DontSortPrimitives", &settings->DontSortPrimitives, false);
    GetRegistryDwordValue("TexelAdjustMode", (DWORD *)&settings->TexelAdjustMode, 2);
    GetRegistryDwordValue("NearestAdjustment", (DWORD *)&settings->NearestAdjustment, 16);
    GetRegistryDwordValue("LinearAdjustment", (DWORD *)&settings->LinearAdjustment, 128);
    GetRegistryBoolValue("FlipBroken", &settings->FlipBroken, false);
    GetRegistryBoolValue("DisableFMV", &settings->DisableFMV, false);
    if (settings->RenderMode <= 0 || settings->RenderMode >= 3)
        settings->RenderMode = 1;
    if (settings->AspectMode < 0 || settings->AspectMode >= 3)
        settings->AspectMode = 0;
    if (settings->TexelAdjustMode < 0 || settings->TexelAdjustMode > 3)
        settings->TexelAdjustMode = 2;
    if (settings->NearestAdjustment < 0)
        settings->NearestAdjustment = 0;
    if (settings->NearestAdjustment > 256)
        settings->NearestAdjustment = 256;
    if (settings->LinearAdjustment < 0)
        settings->LinearAdjustment = 0;
    if (settings->LinearAdjustment > 256)
        settings->LinearAdjustment = 256;
    GetRegistryBoolValue("PerspectiveCorrect", &settings->PerspectiveCorrect, settings->RenderMode != 1);
    body = &((SETUP_ADAPTER_ITEM *)settings->PreferredDisplayAdapter)->body;
    modeList = &body->hwDispModeList;
    if (settings->RenderMode != 2)
        modeList = &body->swDispModeList;
    if (settings->RenderMode == 1)
        target.bpp = 8;
    target.vga = 0;
    mode = modeList->head;
    if (mode)
    {
        while (mode->body.CompareVideoModes(&target))
        {
            mode = mode->next;
            if (!mode)
            {
                mode = modeList->tail;
                break;
            }
        }
    }
    settings->VideoMode = mode;
    CloseGameRegistryKey();
    return IsNewRegistryKeyCreated() ? 2 : 1;
}

extern "C" void SE_DefaultGraphicsSettings(void)
{
    SETUP_ADAPTER_NODE *adapter;
    SETUP_ADAPTER_NODE *swAdapter;
    SETUP_ADAPTER_NODE *hwAdapter;
    SETUP_MODE_LIST *modeList;
    SETUP_MODE_NODE *mode;
    DISPLAY_MODE target;

    hwAdapter = NULL;
    swAdapter = NULL;
    for (adapter = DisplayAdapterList.head; adapter; adapter = adapter->next)
    {
        if (!adapter->lpAdapterGuid)
            swAdapter = adapter;
        if (adapter->hwRenderSupported && (swAdapter != adapter || !hwAdapter))
        {
            hwAdapter = adapter;
            ChangedAppSettings.PerspectiveCorrect = adapter->PerspectiveCorrectSupported;
            ChangedAppSettings.Dither = adapter->DitherSupported;
            ChangedAppSettings.ZBuffer = adapter->ZBufferSupported;
            ChangedAppSettings.BilinearFiltering = adapter->LinearFilterSupported;
        }
        if (!adapter)
            break;
    }
    if (hwAdapter)
    {
        ChangedAppSettings.PreferredDisplayAdapter = hwAdapter;
        ChangedAppSettings.RenderMode = 2;
    }
    else
    {
        ChangedAppSettings.PreferredDisplayAdapter = swAdapter;
        ChangedAppSettings.RenderMode = 1;
        ChangedAppSettings.PerspectiveCorrect = true;
    }
    ChangedAppSettings.FullScreen = true;
    ChangedAppSettings.WindowWidth = 512;
    ChangedAppSettings.WindowHeight = 384;
    if (!ChangedAppSettings.PreferredDisplayAdapter)
        return;
    if (ChangedAppSettings.RenderMode == 2)
        modeList = &((SETUP_ADAPTER_NODE *)ChangedAppSettings.PreferredDisplayAdapter)->hwDispModeList;
    else
        modeList = &((SETUP_ADAPTER_NODE *)ChangedAppSettings.PreferredDisplayAdapter)->swDispModeList;
    mode = modeList->head;
    target.width = 640;
    target.height = 480;
    target.bpp = 16;
    target.vga = 0;
    if (!mode)
    {
        ChangedAppSettings.VideoMode = mode;
        return;
    }
    while (mode->body.CompareVideoModes(&target))
    {
        mode = mode->next;
        if (!mode)
        {
            mode = modeList->tail;
            break;
        }
    }
    ChangedAppSettings.VideoMode = mode;
}

#include "ds.h"

void WinSndStart(HWND hWnd);
LPVOID UT_LoadResource(LPCSTR name, LPCSTR type);
bool WinSndMakeSample(int nSample, WAVEFORMATEX *pWF, void *pWaveData, DWORD dwWaveLength);

extern "C" {
extern HWND SoundDialogHandle;
void WinSndFinish(void);
}

extern "C" bool SE_SoundTestStart(void)
{
    uint8 *waveFile;
    WAVEFORMATEX format;

    try
    {
        WinSndStart(SoundDialogHandle);
    }
    catch (...)
    {
        return false;
    }
    waveFile = (uint8 *)UT_LoadResource(MAKEINTRESOURCE(163), "WAVE");
    format = *(WAVEFORMATEX *)&waveFile[0x14];
    format.cbSize = 0;
    return WinSndMakeSample(0, &format, &waveFile[0x2c], *(DWORD *)&waveFile[0x28]) ? true : false;
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

extern "C" {
extern bool initOnce;
extern bool IsSetupDialogCentered;
extern char SetupDialogName[];
BOOL CALLBACK SE_OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SE_GraphicsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SE_SoundDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SE_ControlsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SE_AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SE_RegisterSetupWindowClass(void);
}

extern "C" bool SE_ShowSetupDialog(HWND hParent, bool isDefault)
{
    HINSTANCE hInst;
    PROPSHEETHEADER header;
    PROPSHEETPAGE pages[5];

    ChangedAppSettings = SavedAppSettings;
    if (isDefault)
    {
        SE_DefaultGraphicsSettings();
        ChangedAppSettings.SoundEnabled = true;
    }
    memset(pages, 0, sizeof(pages));
    hInst = GhInstance;
    pages[0].dwSize = sizeof(PROPSHEETPAGE);
    memset(&header, 0, sizeof(header));
    pages[0].dwFlags = 0;
    pages[0].hInstance = hInst;
    pages[0].pszTemplate = MAKEINTRESOURCE(110);
    pages[0].pfnDlgProc = (DLGPROC)SE_OptionsDlgProc;
    pages[1].dwSize = sizeof(PROPSHEETPAGE);
    pages[1].dwFlags = 0;
    pages[1].hInstance = hInst;
    pages[1].pszTemplate = MAKEINTRESOURCE(107);
    pages[1].pfnDlgProc = (DLGPROC)SE_GraphicsDlgProc;
    pages[2].dwSize = sizeof(PROPSHEETPAGE);
    pages[2].dwFlags = 0;
    pages[2].hInstance = hInst;
    pages[2].pszTemplate = MAKEINTRESOURCE(108);
    pages[2].pfnDlgProc = (DLGPROC)SE_SoundDlgProc;
    pages[3].dwSize = sizeof(PROPSHEETPAGE);
    pages[3].dwFlags = 0;
    pages[3].hInstance = hInst;
    pages[3].pszTemplate = MAKEINTRESOURCE(109);
    pages[3].pfnDlgProc = (DLGPROC)SE_ControlsDlgProc;
    pages[4].dwSize = sizeof(PROPSHEETPAGE);
    pages[4].dwFlags = 0;
    pages[4].hInstance = hInst;
    pages[4].pszTemplate = MAKEINTRESOURCE(144);
    pages[4].pfnDlgProc = (DLGPROC)SE_AdvancedDlgProc;
    header.dwSize = sizeof(PROPSHEETHEADER);
    header.dwFlags = PSH_USECALLBACK | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEHICON;
    header.hwndParent = hParent;
    header.hInstance = hInst;
    header.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(100), IMAGE_ICON, 16, 16, 0);
    header.pszCaption = SetupDialogName;
    header.nPages = 5;
    header.nStartPage = 1;
    header.ppsp = pages;
    header.pfnCallback = SE_PropSheetCallback;
    if (!initOnce)
    {
        InitCommonControls();
        SE_RegisterSetupWindowClass();
        initOnce = true;
    }
    IsSetupDialogCentered = false;
    if (PropertySheet(&header) != 1)
        return false;
    SavedAppSettings = ChangedAppSettings;
    return true;
}

extern "C" {
extern bool Var51bc88;
extern SETUP_ADAPTER_NODE *SE_DisplayAdapter;
extern DISPLAY_MODE SE_FullScreenMode;
void SE_GraphicsDlgInit(HWND hwndDlg);
void SE_GraphicsDlgUpdate(HWND hwndDlg);
void SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg);
void SE_GraphicsAdapterSet(HWND hwndDlg, void *adapter);
void WinVidSetDisplayAdapter(void *adapter);
}

void UT_CenterWindow(HWND hWnd);

extern "C" BOOL CALLBACK SE_GraphicsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool isCheck;
    SETUP_MODE_NODE *mode;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        GraphicsDialogHandle = hwndDlg;
        Var51bc88 = false;
        if (!IsSetupDialogCentered)
        {
            UT_CenterWindow(GetParent(hwndDlg));
            IsSetupDialogCentered = true;
        }
        SE_GraphicsDlgInit(hwndDlg);
        return TRUE;
    case WM_DESTROY:
        GraphicsDialogHandle = NULL;
        break;
    case WM_DISPLAYCHANGE:
        if (!Var51bc88)
        {
            if (SE_DisplayAdapter)
                WinVidSetDisplayAdapter(&SE_DisplayAdapter->lpAdapterGuid);
            SE_GraphicsDlgUpdate(hwndDlg);
        }
        break;
    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hwndDlg, 1034))
            SE_GraphicsDlgUpdate(hwndDlg);
        break;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            isCheck = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
            switch (LOWORD(wParam))
            {
            case 1045:
                ChangedAppSettings.RenderMode = 1;
                SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1047:
                ChangedAppSettings.RenderMode = 2;
                SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1015:
                ChangedAppSettings.PerspectiveCorrect = isCheck;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1058:
                ChangedAppSettings.Dither = isCheck;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1017:
                ChangedAppSettings.ZBuffer = isCheck;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1018:
                ChangedAppSettings.BilinearFiltering = isCheck;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1048:
                ChangedAppSettings.TripleBuffering = isCheck;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1030:
                ChangedAppSettings.FullScreen = true;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1031:
                ChangedAppSettings.FullScreen = false;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1060:
                ChangedAppSettings.AspectMode = 0;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1061:
                ChangedAppSettings.AspectMode = 1;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1062:
                ChangedAppSettings.AspectMode = 2;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1029:
                Var51bc88 = true;
                if (SE_GraphicsTest())
                    PostMessage(SE_PropSheetWndHandle, PSM_PRESSBUTTON, PSBTN_CANCEL, 0);
                Var51bc88 = false;
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            case 1112:
                SE_DefaultGraphicsSettings();
                SE_GraphicsDlgInit(hwndDlg);
                break;
            default:
                SE_GraphicsDlgUpdate(hwndDlg);
                break;
            }
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            mode = (SETUP_MODE_NODE *)SendMessage((HWND)lParam, CB_GETITEMDATA, SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
            if (LOWORD(wParam) == 1000)
            {
                ChangedAppSettings.VideoMode = mode;
                SE_FullScreenMode = mode->body;
            }
            else if (LOWORD(wParam) == 1025)
            {
                SE_GraphicsAdapterSet(hwndDlg, mode);
                SE_GraphicsDlgUpdate(hwndDlg);
            }
        }
        break;
    }
    return FALSE;
}

extern "C" {
extern DISPLAY_MODE SE_FullScreenMode;
extern char RegistryClass[];
}

extern "C" void SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg)
{
    HWND combo;
    WPARAM index, selectedIndex;
    SETUP_MODE_LIST *modeList;
    SETUP_MODE_NODE *mode, *selected;
    SETUP_ADAPTER_NODE *adapter;
    LPCTSTR colorString;
    char buf[256];

    combo = GetDlgItem(hwndDlg, 1000);
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    adapter = (SETUP_ADAPTER_NODE *)ChangedAppSettings.PreferredDisplayAdapter;
    if (ChangedAppSettings.RenderMode == 2)
        modeList = &adapter->hwDispModeList;
    else
        modeList = &adapter->swDispModeList;
    if (ChangedAppSettings.RenderMode == 1)
        SE_FullScreenMode.bpp = 8;
    SE_FullScreenMode.vga = 0;
    mode = modeList->head;
    if (!mode) {
        selected = NULL;
    } else {
        while (mode->body.CompareVideoModes(&SE_FullScreenMode)) {
            mode = mode->next;
            if (!mode) {
                mode = modeList->tail;
                break;
            }
        }
        selected = mode;
    }
    if (modeList->count) {
        selectedIndex = 0;
        colorString = NULL;
        for (mode = modeList->head; mode; mode = mode->next) {
            switch (mode->body.vga) {
            case 1:
                colorString = "256 Color";
                break;
            case 0:
                if (mode->body.bpp == 16)
                    colorString = "High Color";
                else if (mode->body.bpp == 24)
                    colorString = "True Color";
                else if (mode->body.bpp == 32)
                    colorString = "True Color";
                break;
            case 2:
                colorString = "Mode X";
                break;
            case 3:
                colorString = "Standard VGA";
                break;
            default:
                colorString = RegistryClass;
                break;
            }
            sprintf(buf, "%dx%d %s", mode->body.width, mode->body.height, colorString);
            index = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage(combo, CB_SETITEMDATA, index, (LPARAM)mode);
            if (mode == selected)
                selectedIndex = index;
            if (!mode)
                break;
        }
        SendMessage(combo, CB_SETCURSEL, selectedIndex, 0);
        selected = (SETUP_MODE_NODE *)SendMessage(combo, CB_GETITEMDATA, selectedIndex, 0);
        ChangedAppSettings.VideoMode = selected;
        if (selected)
            SE_FullScreenMode = selected->body;
    }
    EnableWindow(combo, (int)modeList->count > 1);
}

extern "C" void SE_GraphicsAdapterSet(HWND hwndDlg, void *adapter)
{
    ChangedAppSettings.PreferredDisplayAdapter = adapter;
    SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
}

extern "C" {
extern SETUP_ADAPTER_NODE *SE_DisplayAdapter;
extern DISPLAY_MODE SE_FullScreenMode;
void WinVidSetDisplayAdapter(void *adapter);
void SE_GraphicsDlgUpdate(HWND hwndDlg);
}

extern "C" void SE_GraphicsDlgInit(HWND hwndDlg)
{
    HWND combo;
    HWND slider;
    WPARAM index;
    WPARAM selected;
    SETUP_ADAPTER_NODE *node;
    GUID **body;
    void *adapter;
    int count;

    combo = GetDlgItem(hwndDlg, 1025);
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    selected = 0;
    SE_DisplayAdapter = NULL;
    for (node = DisplayAdapterList.head; node; node = node->next)
    {
        body = &node->lpAdapterGuid;
        index = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)node->driverDescription);
        SendMessage(combo, CB_SETITEMDATA, index, (LPARAM)node);
        if (node == ChangedAppSettings.PreferredDisplayAdapter)
            selected = index;
        if (!*body)
            SE_DisplayAdapter = node;
        WinVidSetDisplayAdapter(body);
        if (!node)
            break;
    }
    SendMessage(combo, CB_SETCURSEL, selected, 0);
    count = DisplayAdapterList.count;
    EnableWindow(combo, count > 0);
    if (count)
    {
        adapter = (void *)SendMessage(combo, CB_GETITEMDATA, selected, 0);
        if (ChangedAppSettings.VideoMode)
        {
            SE_FullScreenMode = ((SETUP_MODE_NODE *)ChangedAppSettings.VideoMode)->body;
        }
        else
        {
            SE_FullScreenMode.bpp = 0;
            SE_FullScreenMode.width = 640;
            SE_FullScreenMode.height = 480;
            SE_FullScreenMode.vga = 0;
        }
        SE_GraphicsAdapterSet(hwndDlg, adapter);
    }
    slider = GetDlgItem(hwndDlg, 1034);
    SendMessage(slider, TBM_SETPOS, TRUE, (ChangedAppSettings.WindowWidth - 320) >> 5);
    SE_GraphicsDlgUpdate(hwndDlg);
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

extern "C" {
extern HWND ControlsDialogHandle;
extern HWND Var51bcf8;
void Func457470(HWND hWnd, UINT uMsg, WPARAM wParam);
void SE_AdvancedDlgInit(HWND hwndDlg);
void SE_AdvancedDlgUpdate(HWND hwndDlg);
}

extern "C" BOOL CALLBACK SE_AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool isChecked;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ControlsDialogHandle = hwndDlg;
        Var51bcf8 = GetDlgItem(hwndDlg, 1110);
        SE_AdvancedDlgInit(hwndDlg);
        return TRUE;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            isChecked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
            switch (LOWORD(wParam))
            {
            case 1103:
                ChangedAppSettings.Disable16BitTextures = isChecked;
                break;
            case 1104:
                ChangedAppSettings.DontSortPrimitives = isChecked;
                break;
            case 1111:
                ChangedAppSettings.DisableFMV = isChecked;
                break;
            case 1107:
                ChangedAppSettings.TexelAdjustMode = 0;
                break;
            case 1108:
                ChangedAppSettings.TexelAdjustMode = 1;
                break;
            case 1109:
                ChangedAppSettings.TexelAdjustMode = 2;
                break;
            }
            SE_AdvancedDlgUpdate(hwndDlg);
        }
        break;
    }
    Func457470(Var51bcf8, uMsg, wParam);
    return FALSE;
}

inline void SE_CheckButton(HWND hwndDlg, int id, bool check)
{
    CheckDlgButton(hwndDlg, id, check != 0);
}

extern "C" void SE_AdvancedDlgUpdate(HWND hwndDlg)
{
    SE_CheckButton(hwndDlg, 1103, ChangedAppSettings.Disable16BitTextures);
    SE_CheckButton(hwndDlg, 1104, ChangedAppSettings.DontSortPrimitives);
    SE_CheckButton(hwndDlg, 1111, ChangedAppSettings.DisableFMV);
    SE_CheckButton(hwndDlg, 1107, ChangedAppSettings.TexelAdjustMode == 0);
    SE_CheckButton(hwndDlg, 1108, ChangedAppSettings.TexelAdjustMode == 1);
    SE_CheckButton(hwndDlg, 1109, ChangedAppSettings.TexelAdjustMode == 2);
}

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
extern LPCSTR String_Software3d;
extern LPCSTR String_Hardware3d;
extern LPCSTR String_FullScreen;
extern LPCSTR String_Windowed;
extern LPCSTR String_ZBuffered;
extern LPCSTR String_BilinearFiltered;
extern LPCSTR String_Dithered;
extern LPCSTR String_TripleBuffered;
extern LPCSTR String_PerspectiveCorrect;
extern LPCSTR String_None;
extern LPCSTR String_NA;
extern LPCSTR String_Enabled;
extern LPCSTR String_Disabled;
extern LPCSTR String_Camera;
extern LPCSTR Strings_Aspect[];
}

typedef struct
{
    uint8 pad[0x1c];
    LPSTR description;
} SETUP_DEVICE;

typedef struct
{
    uint8 pad[8];
    int width;
    int height;
    int bpp;
} SETUP_VIDEO_MODE;

extern "C" void SE_OptionsDlgUpdate(HWND hwndDlg)
{
    LPSTR str;
    char buffer[256];
    bool isNext;

    if (ChangedAppSettings.PreferredDisplayAdapter)
    {
        str = ((SETUP_DEVICE *)ChangedAppSettings.PreferredDisplayAdapter)->description;
        SetDlgItemText(hwndDlg, 1077, str);
        if (ChangedAppSettings.RenderMode == 1)
            str = (LPSTR)String_Software3d;
        else
            str = (LPSTR)String_Hardware3d;
        SetDlgItemText(hwndDlg, 1078, str);
        if (ChangedAppSettings.FullScreen && ChangedAppSettings.VideoMode)
        {
            SETUP_VIDEO_MODE *mode = (SETUP_VIDEO_MODE *)ChangedAppSettings.VideoMode;
            sprintf(buffer, String_FullScreen, mode->width, mode->height, mode->bpp);
        }
        else
        {
            sprintf(buffer, String_Windowed, ChangedAppSettings.WindowWidth, ChangedAppSettings.WindowHeight, Strings_Aspect[ChangedAppSettings.AspectMode]);
        }
        SetDlgItemText(hwndDlg, 1079, buffer);
        str = buffer;
        isNext = false;
        SE_OptionsStrCat(&str, ChangedAppSettings.ZBuffer, &isNext, String_ZBuffered);
        SE_OptionsStrCat(&str, ChangedAppSettings.BilinearFiltering, &isNext, String_BilinearFiltered);
        SE_OptionsStrCat(&str, ChangedAppSettings.Dither, &isNext, String_Dithered);
        SE_OptionsStrCat(&str, ChangedAppSettings.TripleBuffering, &isNext, String_TripleBuffered);
        SE_OptionsStrCat(&str, ChangedAppSettings.PerspectiveCorrect, &isNext, String_PerspectiveCorrect);
        *str = 0;
        SetDlgItemText(hwndDlg, 1080, buffer);
    }
    else
    {
        SetDlgItemText(hwndDlg, 1077, String_None);
        SetDlgItemText(hwndDlg, 1078, String_NA);
        SetDlgItemText(hwndDlg, 1079, String_NA);
        SetDlgItemText(hwndDlg, 1080, String_NA);
    }
    if (ChangedAppSettings.PreferredSoundAdapter)
        str = ((SETUP_DEVICE *)ChangedAppSettings.PreferredSoundAdapter)->description;
    else
        str = (LPSTR)String_None;
    SetDlgItemText(hwndDlg, 1081, str);
    SetDlgItemText(hwndDlg, 1082, ChangedAppSettings.SoundEnabled ? String_Enabled : String_Disabled);
    SetDlgItemText(hwndDlg, 1085, ChangedAppSettings.LaraMic ? "Lara" : String_Camera);
    if (ChangedAppSettings.PreferredJoystick)
        str = ((SETUP_DEVICE *)ChangedAppSettings.PreferredJoystick)->description;
    else
        str = (LPSTR)String_None;
    SetDlgItemText(hwndDlg, 1084, str);
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

typedef struct SETUP_SOUND_NODE
{
    struct SETUP_SOUND_NODE *next;
    uint8 pad[0x18];
    LPCSTR description;
} SETUP_SOUND_NODE;

extern "C" void SE_SoundDlgInit(HWND hwndDlg)
{
    HWND combo;
    WPARAM index;
    WPARAM selected;
    SETUP_SOUND_NODE *node;
    int count;

    combo = GetDlgItem(hwndDlg, 1052);
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    selected = 0;
    for (node = (SETUP_SOUND_NODE *)SoundAdapterList.head; node; node = !node ? NULL : node->next)
    {
        index = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)node->description);
        SendMessage(combo, CB_SETITEMDATA, index, (LPARAM)node);
        if (node == ChangedAppSettings.PreferredSoundAdapter)
            selected = index;
    }
    SendMessage(combo, CB_SETCURSEL, selected, 0);
    count = SoundAdapterList.count;
    EnableWindow(combo, count > 1);
    if (count)
        SE_SoundAdapterSet(hwndDlg, (void *)SendMessage(combo, CB_GETITEMDATA, selected, 0));
    SE_SoundDlgUpdate(hwndDlg);
}

extern "C" {
extern HWND SoundImageHandle;
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

typedef struct SETUP_JOYSTICK_NODE
{
    struct SETUP_JOYSTICK_NODE *next;
    uint8 pad[0x18];
    LPCSTR description;
} SETUP_JOYSTICK_NODE;

extern "C" void SE_ControlsDlgInit(HWND hwndDlg)
{
    HWND combo;
    WPARAM index;
    WPARAM selected;
    SETUP_JOYSTICK_NODE *node;
    int count;

    combo = GetDlgItem(hwndDlg, 1055);
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    selected = 0;
    for (node = (SETUP_JOYSTICK_NODE *)JoystickList.head; node; node = !node ? NULL : node->next)
    {
        index = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)node->description);
        SendMessage(combo, CB_SETITEMDATA, index, (LPARAM)node);
        if (node == ChangedAppSettings.PreferredJoystick)
            selected = index;
    }
    SendMessage(combo, CB_SETCURSEL, selected, 0);
    count = JoystickList.count;
    EnableWindow(combo, count > 0);
    if (count)
        SE_ControlsJoystickSet(hwndDlg, (void *)SendMessage(combo, CB_GETITEMDATA, selected, 0));
    SE_ControlsDlgUpdate(hwndDlg);
}
