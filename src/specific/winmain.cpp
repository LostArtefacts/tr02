// cflags: /GX
#include "include.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "init_result.h"

extern "C" {
extern char StringToShow[128];
void WinInFinish(void);
void WinSndFinish(void);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
LPCTSTR DecodeErrorMessage(DWORD errorCode);
extern char RenderInitFailedFormat[];
}
void RenderFinish(bool reset);

void UT_ErrorBox(char *text, void *parent);

extern "C" void RenderErrorBox(int errorCode)
{
    char text[128];

    sprintf(text, RenderInitFailedFormat, DecodeErrorMessage(errorCode));
    UT_ErrorBox(text, NULL);
}

extern "C" {
bool CD_Init(void);
bool WinVidInit(void);
BOOL Direct3DInit(void);
bool RenderInit(void);
bool InitTextures(void);
bool DS_Init(void);
bool DI_Init(void);
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
    if (WinVidInit() && Direct3DInit() && RenderInit() && InitTextures() && DS_Init() && DI_Init() && TIME_Init() && HWR_Init() && BGND_Init())
        return 1;
    return 0;
}

extern "C" {
void WinCleanupStub1(void);
void WinCleanupStub2(void);
void WinCleanupStub3(void);
void WinCleanupStub4(void);
void WinCleanupStub5(void);
void WinVidFreeWindow(void);
void CD_Cleanup(void);
}

void WinCleanup(void)
{
    WinCleanupStub1();
    WinCleanupStub2();
    WinCleanupStub3();
    WinCleanupStub4();
    WinCleanupStub5();
    WinVidFreeWindow();
    CD_Cleanup();
}

void WinVidStart(void);
void RenderStart(bool reset);
void WinSndStart(HWND hWnd);
void WinInStart(void);

extern "C" int WinGameStart(void)
{
    try {
        WinVidStart();
        RenderStart(true);
        WinSndStart(NULL);
        WinInStart();
    } catch (InitResult error) {
        return error;
    }
    return 0;
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
void WinCleanup(void);

extern "C" void S_ExitSystem(char *message)
{
    ShutdownGame();
    strcpy(StringToShow, message);
    WinGameFinish();
    WinCleanup();
    exit(1);
}

#ifdef TR2_EXE
char RenderInitFailedFormat[] = "Render init failed with \"%s\"";

extern "C" {
char LszVersion[] = "Prerelease Dec 10 1997";
char LszVersionRegisteredTo[] = "Public Release";
char LszReleaseCode[] = "0000";
char LszCopyrightMessage[] = "Copyright (c) 1997 Core Design Ltd.";
char *LaszCoderNames[] = {"Andrew", "Gavin", "Jason"};

void WinCleanupStub1(void)
{
}

void WinCleanupStub2(void)
{
}

void WinCleanupStub3(void)
{
}

void WinCleanupStub4(void)
{
}

void WinCleanupStub5(void)
{
}
}

#endif
