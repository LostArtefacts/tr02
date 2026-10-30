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

extern "C" bool WinSndMakeSample(int nSample, WAVEFORMATEX *pWF, void *pWaveData, DWORD dwWaveLength)
{
    DSBUFFERDESC desc;
    void *pBlock;
    DWORD dwBlockLength;

    if (!IsSoundEnabled)
        return false;
    if (nSample >= ArraySize(SampleBuffers))
        return false;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_LOCSOFTWARE;
    desc.dwBufferBytes = dwWaveLength;
    desc.lpwfxFormat = pWF;
    desc.dwReserved = 0;
    if (DX_TRY(DSound->CreateSoundBuffer(&desc, SampleBuffers + nSample, NULL)))
        return false;
    if (DX_TRY(SampleBuffers[nSample]->Lock(0, dwWaveLength, &pBlock, &dwBlockLength, NULL, NULL, 0)))
        return false;
    memcpy(pBlock, pWaveData, dwBlockLength);
    if (DX_TRY(SampleBuffers[nSample]->Unlock(pBlock, dwBlockLength, NULL, 0)))
        return false;
    SampleFreqs[nSample] = pWF->nSamplesPerSec;
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

extern "C" int WinSndGetFreeChannelIndex();

