#ifndef UTILS_H
#define UTILS_H

void UT_MemBlt(uint8 *pDest, int nDX, int nDY, int nDW, int nDH, int nDStride, uint8 *pSrc, int nSX, int nSY, int nSStride);
void UT_InitAccurateTimer();
double UT_GetAccurateTimer();
void UT_CenterWindow(HWND hWnd);
char *UT_FindArg(char *arg);
void UT_ErrorBox(char *szString, HWND hWndOwner = 0);
void UT_ErrorBox(int nID, HWND hWndOwner = 0);

#endif
