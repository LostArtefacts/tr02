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
