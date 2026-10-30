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
