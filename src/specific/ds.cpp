#include "include.h"
#include <dsound.h>
#include "global.h"
#include "ds.h"

extern "C" LPDIRECTSOUND DSound;
extern "C" LPDIRECTSOUNDBUFFER SampleBuffers[256];
extern "C" bool IsSoundEnabled;
extern "C" int ChannelSamples[32];
extern "C" LPDIRECTSOUNDBUFFER ChannelBuffers[32];
extern "C" int SampleFreqs[256];

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" void WinSndFreeAllSamples()
{
    if (IsSoundEnabled) {
        for (int i = 0; i < ArraySize(SampleBuffers); ++i) {
            if (SampleBuffers[i]) {
                SampleBuffers[i]->Release();
                SampleBuffers[i] = 0;
            }
        }
    }
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

extern "C" int WinSndGetFreeChannelIndex();

