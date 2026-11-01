#include "global.h"
#include "time.h"

extern double LdSync, LdFreq;

extern "C" void TIME_Reset()
{
    LARGE_INTEGER fq;

    QueryPerformanceCounter(&fq);
    LdSync = (double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff;
    LdSync /= LdFreq;
}

bool TIME_Init()
{
    LARGE_INTEGER fq;

    if (!QueryPerformanceFrequency(&fq))
        return false;
    LdFreq = (double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff;
    LdFreq /= 60.0;
    TIME_Reset();
    return true;
}

int Sync(void)
{
    int nFrames;
    double dCounter;
    LARGE_INTEGER ct;

    QueryPerformanceCounter(&ct);
    dCounter = (double)ct.LowPart + (double)ct.HighPart * (double)0xffffffff;
    dCounter /= LdFreq;
    nFrames = long(dCounter) - long(LdSync);
    LdSync = dCounter;
    return nFrames;
}

#ifdef TR2_EXE
double LdSync;
double LdFreq;
#endif
