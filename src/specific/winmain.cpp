#include "include.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
extern char StringToShow[128];
void WinInFinish(void);
void WinSndFinish(void);
void RenderFinish(bool reset);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
LPCTSTR DecodeErrorMessage(DWORD errorCode);
extern char RenderInitFailedFormat[];
}

void UT_ErrorBox(char *text, void *parent);

extern "C" void RenderErrorBox(int errorCode)
{
    char text[128];

    sprintf(text, RenderInitFailedFormat, DecodeErrorMessage(errorCode));
    UT_ErrorBox(text, NULL);
}

extern "C" void WinGameFinish(void)
{
    WinInFinish();
    WinSndFinish();
    RenderFinish(true);
    WinVidFinish();
    WinVidHideGameWindow();
    if (*StringToShow)
        MessageBoxA(NULL, StringToShow, NULL, MB_ICONWARNING);
}

extern "C" void ShutdownGame(void);
extern "C" void WinCleanup(void);

extern "C" void S_ExitSystem(char *message)
{
    ShutdownGame();
    strcpy(StringToShow, message);
    WinGameFinish();
    WinCleanup();
    exit(1);
}

extern "C" {
bool CD_Init(void);
bool WinVidInit(void);
BOOL Direct3DInit(void);
bool RenderInit(void);
bool InitTextures(void);
bool Func448050(void);
bool Func447450(void);
bool BGND_Init(void);
}

void UT_InitAccurateTimer(void);
bool TIME_Init(void);
bool HWR_Init(void);

extern "C" int GameInit(bool skipCDInit)
{
    if (!skipCDInit && !CD_Init())
        return 2;
    UT_InitAccurateTimer();
    if (WinVidInit() && Direct3DInit() && RenderInit() && InitTextures() && Func448050() && Func447450() && TIME_Init() && HWR_Init() && BGND_Init())
        return 1;
    return 0;
}
