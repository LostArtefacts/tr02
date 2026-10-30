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

typedef struct
{
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    uint8 rest[0x2d];
} SETUP_APP_SETTINGS;

extern "C" {
extern SETUP_APP_SETTINGS SavedAppSettings;
extern SETUP_APP_SETTINGS ChangedAppSettings;
void RenderFinish(bool reset);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
bool SE_GraphicsTestStart(void);
}

#include "ds.h"

extern "C" {
void WinSndFinish(void);
bool SE_SoundTestStart(void);
}

extern "C" void SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg);

extern "C" void SE_AdvancedDlgUpdate(HWND hwndDlg);

extern "C" char SetupDialogName[];

