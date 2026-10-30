#include "global.h"
#include "utils.h"
#include <stdio.h>

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

void UT_ErrorBox(char *szString, HWND hWndOwner)
{
    MessageBox(hWndOwner, szString, "Tomb Raider II", MB_ICONERROR);
}

void UT_ErrorBox(int nID, HWND hWndOwner)
{
    char buffer[256];
    if (LoadString(GhInstance, nID, buffer, sizeof buffer))
        UT_ErrorBox(buffer, hWndOwner);
}

extern "C" HKEY HKey;
extern "C" DWORD RegKeyDisposition;
extern "C" char RegistryClass[];

extern "C" BOOL OpenRegistryKey(LPCTSTR lpSubKey)
{
    return RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, RegistryClass, 0, KEY_ALL_ACCESS, NULL, &HKey, &RegKeyDisposition) == ERROR_SUCCESS;
}

extern "C" bool IsNewRegistryKeyCreated(void)
{
    return RegKeyDisposition == REG_CREATED_NEW_KEY;
}

extern "C" LONG CloseRegistryKey(void)
{
    return RegCloseKey(HKey);
}

extern "C" LONG SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value)
{
    return RegSetValueEx(HKey, lpValueName, 0, REG_DWORD, (BYTE *)&value, sizeof(DWORD));
}

extern "C" LONG SetRegistryBoolValue(LPCTSTR lpValueName, bool value)
{
    DWORD dword = value;
    return RegSetValueEx(HKey, lpValueName, 0, REG_DWORD, (BYTE *)&dword, sizeof(DWORD));
}

extern "C" LONG SetRegistryStringValue(LPCTSTR lpValueName, LPCTSTR value, int length);

extern "C" LONG SetRegistryFloatValue(LPCTSTR lpValueName, double value)
{
    char buffer[64];
    int length = sprintf(buffer, "%.5f", value);
    return SetRegistryStringValue(lpValueName, buffer, length);
}

extern "C" LONG SetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize)
{
    if (value)
        return RegSetValueEx(HKey, lpValueName, 0, REG_BINARY, value, valueSize);
    return RegDeleteValue(HKey, lpValueName);
}

void UT_MemBlt(uint8 *pDest, int nDX, int nDY, int nDW, int nDH, int nDStride, uint8 *pSrc, int nSX, int nSY, int nSStride)
{
    pDest += (nDX + nDY * nDStride);
    pSrc += (nSX + nSY * nSStride);
    while (nDH--)
    {
        memcpy(pDest, pSrc, nDW);
        pDest += nDStride;
        pSrc += nSStride;
    }
}
