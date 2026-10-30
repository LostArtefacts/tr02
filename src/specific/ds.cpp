#include <windows.h>
#include <dsound.h>
#include "global.h"
#include "ds.h"

extern "C" LPDIRECTSOUNDBUFFER ChannelBuffers[32];

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

bool DS_IsChannelPlaying(int nChannel)
{
    LPDIRECTSOUNDBUFFER pBuffer;
    DWORD dwStatus;

    pBuffer = ChannelBuffers[nChannel];
    if (!pBuffer)
        return false;
    if (DX_TRY(pBuffer->GetStatus(&dwStatus)))
        return false;
    if (dwStatus & DSBSTATUS_PLAYING)
        return true;
    pBuffer->Release();
    ChannelBuffers[nChannel] = 0;
    return false;
}
