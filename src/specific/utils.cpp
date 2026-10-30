#include "global.h"
#include "utils.h"
#include <stdio.h>

extern double LdPerformanceDivisor;
extern double LdInitialTimer;
extern DWORD LdwInitialTimer;

extern "C" LPVOID UT_LoadResource(LPCSTR name, LPCSTR type)
{
    HRSRC res = FindResource(GhInstance, name, type);
    if (!res)
        return NULL;
    HGLOBAL data = LoadResource(GhInstance, res);
    if (!data)
        return NULL;
    return LockResource(data);
}

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

extern "C" const char GuidFormat[];
extern "C" const char HexDigits[];
extern "C" char GuidString[];

extern "C" LPCTSTR GuidBinaryToString(GUID *guid)
{
    char *p = GuidString;
    for (unsigned int i = 0; i < 21; i++)
    {
        if (GuidFormat[i] < 16)
        {
            *p++ = HexDigits[((uint8 *)guid)[GuidFormat[i]] >> 4];
            *p++ = HexDigits[((uint8 *)guid)[GuidFormat[i]] & 15];
        }
        else
            *p++ = GuidFormat[i];
    }
    *p = 0;
    return GuidString;
}

inline char HexCharValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'A' + 10;
    return -1;
}

extern "C" bool GuidStringToBinary(LPCTSTR lpString, GUID *guid)
{
    for (unsigned int i = 0; i < 21; i++)
    {
        char c = GuidFormat[i];
        if (c < 16)
        {
            char hi = HexCharValue(*lpString++);
            if (hi < 0)
                return false;
            char lo = HexCharValue(*lpString++);
            if (lo < 0)
                return false;
            ((uint8 *)guid)[GuidFormat[i]] = hi * 16 + lo;
        }
        else if (*lpString++ != c)
            return false;
    }
    return *lpString == 0;
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

extern "C" LONG SetRegistryStringValue(LPCTSTR lpValueName, LPCTSTR value, int length)
{
    if (value)
    {
        if (length < 0)
            length = strlen(value);
        return RegSetValueEx(HKey, lpValueName, 0, REG_SZ, (BYTE *)value, length + 1);
    }
    return RegDeleteValue(HKey, lpValueName);
}

extern "C" LONG DeleteRegistryValue(LPCTSTR lpValueName)
{
    return RegDeleteValue(HKey, lpValueName);
}

extern "C" bool GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue)
{
    DWORD type;
    DWORD size = sizeof(DWORD);
    if (!RegQueryValueEx(HKey, lpValueName, 0, &type, (BYTE *)pValue, &size) && type == REG_DWORD && size == sizeof(DWORD))
        return true;
    SetRegistryDwordValue(lpValueName, defaultValue);
    *pValue = defaultValue;
    return false;
}

extern "C" bool GetRegistryBoolValue(LPCTSTR lpValueName, bool *pValue, bool defaultValue)
{
    DWORD type;
    DWORD value;
    DWORD size = sizeof(DWORD);
    if (!RegQueryValueEx(HKey, lpValueName, 0, &type, (BYTE *)&value, &size) && type == REG_DWORD && size == sizeof(DWORD))
    {
        *pValue = value != 0;
        return true;
    }
    SetRegistryBoolValue(lpValueName, defaultValue);
    *pValue = defaultValue;
    return false;
}

extern "C" bool GetRegistryStringValue(LPCTSTR lpValueName, LPTSTR value, DWORD maxSize, LPCTSTR defaultValue);

extern "C" bool GetRegistryFloatValue(LPCTSTR lpValueName, double *value, double defaultValue)
{
    char buffer[64];
    if (GetRegistryStringValue(lpValueName, buffer, sizeof(buffer), NULL))
    {
        *value = atof(buffer);
        return true;
    }
    SetRegistryFloatValue(lpValueName, defaultValue);
    *value = defaultValue;
    return false;
}

extern "C" bool GetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize, LPBYTE defaultValue)
{
    DWORD type;
    DWORD size = valueSize;
    if (!RegQueryValueEx(HKey, lpValueName, 0, &type, value, &size) && type == REG_BINARY && valueSize == size)
        return true;
    if (defaultValue)
        SetRegistryBinaryValue(lpValueName, value, valueSize);
    else
        RegDeleteValue(HKey, lpValueName);
    return false;
}

extern "C" bool GetRegistryStringValue(LPCTSTR lpValueName, LPTSTR value, DWORD maxSize, LPCTSTR defaultValue)
{
    DWORD type;
    DWORD size = maxSize;
    if (!RegQueryValueEx(HKey, lpValueName, 0, &type, (BYTE *)value, &size) && type == REG_SZ)
        return true;
    if (defaultValue)
    {
        SetRegistryStringValue(lpValueName, defaultValue, -1);
        int length = strlen(defaultValue) + 1;
        if (length > (int)maxSize)
        {
            length = maxSize - 1;
            value[length] = 0;
        }
        memcpy(value, defaultValue, length);
    }
    else
        RegDeleteValue(HKey, lpValueName);
    return false;
}

extern "C" bool GuidStringToBinary(LPCTSTR lpString, GUID *lpGuid);
extern "C" LPCTSTR GuidBinaryToString(GUID *lpGuid);

extern "C" bool GetRegistryGuidValue(LPCTSTR lpValueName, GUID *value, GUID *defaultValue)
{
    char buffer[40];
    if (GetRegistryStringValue(lpValueName, buffer, 38, NULL) && GuidStringToBinary(buffer, value))
        return true;
    if (defaultValue)
    {
        SetRegistryStringValue(lpValueName, GuidBinaryToString(defaultValue), 37);
        *value = *defaultValue;
    }
    else
        RegDeleteValue(HKey, lpValueName);
    return false;
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

extern "C" void Func457470(HWND hWnd, UINT uMsg, WPARAM wParam)
{
    if (uMsg == WM_PALETTECHANGED || uMsg == WM_QUERYNEWPALETTE)
        SendMessage(hWnd, uMsg, wParam, 0);
}

extern "C" char DialogClassName[];
extern "C" LRESULT CALLBACK SE_SetupWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern "C" bool SE_RegisterSetupWindowClass(void)
{
    WNDCLASS wc;

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = SE_SetupWindowProc;
    wc.hInstance = GhInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = DialogClassName;
    wc.cbWndExtra = 12;
    return RegisterClass(&wc) ? true : false;
}

class BITMAP_RESOURCE
{
public:
    LPBITMAPINFO bmpInfo;
    LPBYTE bmpData;
    HPALETTE hPalette;
    DWORD flags;

    void Release(void);
    void Draw(HDC hdc, int x, int y);
    void UpdatePalette(HWND hWnd, HWND hSender);
    void ChangePalette(HWND hWnd);
};

void BITMAP_RESOURCE::Release(void)
{
    if (hPalette)
    {
        DeleteObject(hPalette);
        hPalette = NULL;
    }
    if ((flags & 1) && bmpData)
    {
        free(bmpData);
        bmpData = NULL;
    }
}

void BITMAP_RESOURCE::Draw(HDC hdc, int x, int y)
{
    if (!hPalette)
        return;
    LPBITMAPINFO info = bmpInfo;
    HPALETTE old = SelectPalette(hdc, hPalette, FALSE);
    RealizePalette(hdc);
    SetDIBitsToDevice(hdc, x, y, info->bmiHeader.biWidth, info->bmiHeader.biHeight, 0, 0, 0, info->bmiHeader.biHeight, bmpData, bmpInfo, DIB_RGB_COLORS);
    SelectPalette(hdc, old, FALSE);
}

void BITMAP_RESOURCE::ChangePalette(HWND hWnd)
{
    if (!hPalette)
        return;
    HDC hdc = GetDC(hWnd);
    HPALETTE old = SelectPalette(hdc, hPalette, FALSE);
    UnrealizeObject(hPalette);
    RealizePalette(hdc);
    SelectPalette(hdc, old, FALSE);
    ReleaseDC(hWnd, hdc);
}
