#include "include.h"
#include "global.h"

extern "C" {
extern char StringToShow[128];
void WinInFinish(void);
void WinSndFinish(void);
void RenderFinish(bool reset);
void WinVidFinish(void);
void WinVidHideGameWindow(void);
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
