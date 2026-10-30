#include "include.h"
#include "global.h"

extern "C" {
extern LPDIRECTDRAW2 DDraw;
extern LPDIRECT3D2 D3D;
bool D3DCreate(void);
void D3DRelease(void);
HRESULT WINAPI Enum3DDevicesCallback(GUID *lpGuid, LPTSTR lpDeviceDescription, LPTSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpContext);
}

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" bool D3DCreate(void)
{
    if (DX_TRY(DDraw->QueryInterface(IID_IDirect3D2, (LPVOID *)&D3D)))
        return false;
    return true;
}

extern "C" BOOL Direct3DInit(void)
{
    return TRUE;
}
