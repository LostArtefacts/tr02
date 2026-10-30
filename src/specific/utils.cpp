#include "global.h"
#include "utils.h"

extern double LdPerformanceDivisor;
extern double LdInitialTimer;
extern DWORD LdwInitialTimer;

void UT_InitAccurateTimer()
{
    LARGE_INTEGER pf;

    if (QueryPerformanceFrequency(&pf))
    {
        LdPerformanceDivisor = 1.0 / double(pf.QuadPart);
        LdInitialTimer = 0;
        LdInitialTimer = UT_GetAccurateTimer();
    }
    else
    {
        LdPerformanceDivisor = 0.0;
        LdwInitialTimer = timeGetTime();
    }
}

double UT_GetAccurateTimer()
{
    LARGE_INTEGER pr;

    if (QueryPerformanceCounter(&pr))
        return LdPerformanceDivisor * (double(pr.QuadPart) - LdInitialTimer);
    else
        return double(timeGetTime() - LdwInitialTimer) / 1000.0;
}

void UT_CenterWindow(HWND hWnd)
{
    RECT rcWnd, rcArea;

    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
    GetWindowRect(hWnd, &rcWnd);
    int x, y;
    x = (rcArea.right + rcArea.left) >> 1;
    y = (rcArea.bottom + rcArea.top) >> 1;
    x -= (rcWnd.right - rcWnd.left) >> 1;
    y -= (rcWnd.bottom - rcWnd.top) >> 1;
    SetWindowPos(hWnd, NULL, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

char *UT_FindArg(char *szArg)
{
    char *pArg = strstr(GpCmdLine, szArg);
    if (!pArg)
        return 0;
    return pArg + strlen(szArg);
}
