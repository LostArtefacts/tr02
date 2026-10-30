#include "include.h"
#include "global.h"

extern "C" {
extern LPCTSTR ErrorStringTable[];
}

extern "C" LPCTSTR DecodeErrorMessage(DWORD errorCode)
{
    return ErrorStringTable[errorCode];
}
