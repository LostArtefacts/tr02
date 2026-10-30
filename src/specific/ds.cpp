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

int DS_StartSample(int nSample, int nVolume, int nPitch, int nPan, DWORD dwFlags)
{
    int nChannel;
    int nFrequency;
    LPDIRECTSOUNDBUFFER pOrigBuffer, pBuffer;

    nChannel = WinSndGetFreeChannelIndex();
    if (nChannel < 0)
        return -1;
    pOrigBuffer = SampleBuffers[nSample];
    if (DX_TRY(DSound->DuplicateSoundBuffer(pOrigBuffer, &pBuffer)))
        return -2;
    nFrequency = (nPitch * SampleFreqs[nSample]) >> 16;
    if (DX_TRY(pBuffer->SetVolume(nVolume)) || DX_TRY(pBuffer->SetFrequency(nFrequency))
        || DX_TRY(pBuffer->SetPan(nPan)) || DX_TRY(pBuffer->SetCurrentPosition(0))
        || DX_TRY(pBuffer->Play(0, 0, dwFlags)))
        return -2;
    ChannelBuffers[nChannel] = pBuffer;
    ChannelSamples[nChannel] = nSample;
    return nChannel;
}

extern "C" int WinSndGetFreeChannelIndex()
{
    int nChannel;

    for (nChannel = 0; nChannel < ArraySize(ChannelBuffers); ++nChannel) {
        if (ChannelBuffers[nChannel] == 0)
            return nChannel;
    }
    for (nChannel = 0; nChannel < ArraySize(ChannelBuffers); ++nChannel) {
        if (!DS_IsChannelPlaying(nChannel))
            return nChannel;
    }
    return -1;
}

void DS_AdjustVolumeAndPan(int nHandle, int nVolume, int nPan)
{
    LPDIRECTSOUNDBUFFER pBuffer;

    if (nHandle < 0)
        return;
    pBuffer = ChannelBuffers[nHandle];
    if (pBuffer) {
        DX_TRY(pBuffer->SetVolume(nVolume));
        DX_TRY(pBuffer->SetPan(nPan));
    }
}

void DS_AdjustPitch(int nHandle, int nPitch)
{
    LPDIRECTSOUNDBUFFER pBuffer;
    int nFrequency;

    if (nHandle < 0)
        return;
    pBuffer = ChannelBuffers[nHandle];
    if (pBuffer) {
        nFrequency = (nPitch * SampleFreqs[ChannelSamples[nHandle]]) >> 16;
        DX_TRY(pBuffer->SetFrequency(nFrequency));
    }
}

void DS_StopSample(int nHandle)
{
    LPDIRECTSOUNDBUFFER pBuffer;

    if (nHandle < 0)
        return;
    pBuffer = ChannelBuffers[nHandle];
    if (pBuffer) {
        DX_TRY(pBuffer->Stop());
        pBuffer->Release();
        ChannelBuffers[nHandle] = 0;
    }
}

void DS_StopAllSamples()
{
    for (int nChannel = 0; nChannel < ArraySize(ChannelBuffers); ++nChannel) {
        LPDIRECTSOUNDBUFFER pBuffer = ChannelBuffers[nChannel];
        if (pBuffer) {
            DX_TRY(pBuffer->Stop());
            pBuffer->Release();
            ChannelBuffers[nChannel] = 0;
        }
    }
}

extern "C" bool WinSndInit(void);

extern "C" bool Func448050(void)
{
    return WinSndInit();
}

extern "C" BOOL CALLBACK DSoundEnumCallback(LPGUID lpGuid, LPCSTR lpDescription, LPCSTR lpModule, LPVOID lpContext);

extern "C" bool DSoundEnumerate(LPVOID lpContext)
{
    if (DX_TRY(DirectSoundEnumerate(DSoundEnumCallback, lpContext)))
        return false;
    return true;
}

extern "C" bool DSoundCreate(LPGUID lpGuid)
{
    if (DX_TRY(DirectSoundCreate(lpGuid, &DSound, NULL)))
        return false;
    return true;
}

extern "C" bool WinSndIsSoundEnabled()
{
    return IsSoundEnabled;
}
