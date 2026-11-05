// cflags: /GX
#include "include.h"
#include "global.h"
#include "winmain.h"
#include <stdlib.h>
#include <eh.h>
#include "se_exception.h"

extern "C" {
extern int AppResultCode;
extern int StopInventory;
extern bool IsGameToExit;
HWND WinVidFindGameWindow(void);
HWND SE_FindSetupDialog(void);
void _cexit(void);
int GameInit(bool setup);
int SE_ReadAppSettings(APP_SETTINGS *settings);
bool SE_ShowSetupDialog(HWND parent, bool firstRun);
void Func4524e0(APP_SETTINGS *settings);
int WinGameStart(void);
void WinGameFinish(void);
void RenderErrorBox(int errorCode);
BOOL GameMain(void);
}

void WinCleanup(void);
extern "C" void SE_TranslateException(unsigned int code, EXCEPTION_POINTERS *pointers);
char *UT_FindArg(char *arg);
void UT_ErrorBox(int id, void *parent);
void UT_ErrorBox(char *text, void *parent);

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND gameWindow;
    HWND setupDialog;
    int rc;
    int initStatus;
    int settingsStatus;
    bool setup;
    bool firstRun;

    GhInstance = hInstance;
    GpCmdLine = lpCmdLine;
    gameWindow = WinVidFindGameWindow();
    if (gameWindow) {
        setupDialog = SE_FindSetupDialog();
        SetForegroundWindow(setupDialog ? setupDialog : gameWindow);
        return 0;
    }
    try {
        _set_se_translator(SE_TranslateException);
        *(volatile bool *)&hInstance = false;
        setup = UT_FindArg("setup") != NULL;
        initStatus = GameInit(setup);
        if (initStatus == 1) {
            settingsStatus = SE_ReadAppSettings(&SavedAppSettings);
            firstRun = settingsStatus == 2;
            if (settingsStatus == 0) {
                *(volatile bool *)&hInstance = true;
            } else if (firstRun || setup) {
                if (!SE_ShowSetupDialog(NULL, firstRun)) {
                    *(volatile bool *)&hInstance = true;
                } else {
                    Func4524e0(&SavedAppSettings);
                    if (setup)
                        *(volatile bool *)&hInstance = true;
                }
            }
            while (!*(volatile bool *)&hInstance) {
                rc = WinGameStart();
                if (!rc) {
                    StopInventory = 0;
                    IsGameToExit = false;
                    GameMain();
                    WinGameFinish();
                    Func4524e0(&SavedAppSettings);
                    *(volatile bool *)&hInstance = true;
                } else {
                    WinGameFinish();
                    RenderErrorBox(rc);
                    if (!SE_ShowSetupDialog(NULL, false))
                        *(volatile bool *)&hInstance = true;
                    else
                        Func4524e0(&SavedAppSettings);
                }
            }
        } else if (initStatus == 0) {
            UT_ErrorBox(1, NULL);
        }
    } catch (char *message) {
        WinGameFinish();
        UT_ErrorBox(message, NULL);
    } catch (SE_Exception exception) {
        WinGameFinish();
        WinCleanup();
        _cexit();
        abort();
    }
    WinCleanup();
    return AppResultCode;
}
