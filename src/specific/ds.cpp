#include "include.h"
#include <dsound.h>
#include "global.h"
#include "ds.h"
#include "../camera.h"

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

struct STRING_FLAGGED {
    LPTSTR lpString;
    bool isPresented;
    STRING_FLAGGED(DWORD size);
    ~STRING_FLAGGED()
    {
        if (isPresented)
            delete[] lpString;
    }
    STRING_FLAGGED &operator=(const STRING_FLAGGED &src)
    {
        if (this != &src) {
            if (!isPresented)
                lpString = new char[strlen(src.lpString) + 1];
            strcpy(lpString, src.lpString);
        }
        return *this;
    }
};

struct SOUND_ADAPTER {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED description;
    STRING_FLAGGED module;
    SOUND_ADAPTER() : description(256), module(256)
    {
    }
};

struct SOUND_ADAPTER_NODE {
    SOUND_ADAPTER_NODE *next;
    SOUND_ADAPTER_NODE *previous;
    SOUND_ADAPTER body;
    SOUND_ADAPTER_NODE(SOUND_ADAPTER_NODE *prevNode, SOUND_ADAPTER_NODE *nextNode) : next(nextNode), previous(prevNode)
    {
    }
};

struct SOUND_ADAPTER_LIST {
    SOUND_ADAPTER_NODE *head;
    SOUND_ADAPTER_NODE *tail;
    DWORD dwCount;
    SOUND_ADAPTER_LIST()
    {
        head = NULL;
        tail = NULL;
        dwCount = 0;
    }
    ~SOUND_ADAPTER_LIST();
};

extern "C" {
extern SOUND_ADAPTER_LIST SoundAdapterList;
extern SOUND_ADAPTER_NODE *PrimarySoundAdapter;
bool DSoundEnumerate(LPVOID lpContext);
}

extern "C" bool WinSndInit(void)
{
    SOUND_ADAPTER_NODE *node;
    SOUND_ADAPTER_NODE *nextNode;

    for (node = SoundAdapterList.head; node; node = nextNode)
    {
        nextNode = node->next;
        delete node;
    }
    SoundAdapterList.head = NULL;
    SoundAdapterList.tail = NULL;
    SoundAdapterList.dwCount = 0;
    if (!DSoundEnumerate(&SoundAdapterList))
        return false;
    PrimarySoundAdapter = NULL;
    for (node = SoundAdapterList.head; node; node = node->next)
    {
        if (!node->body.lpAdapterGuid)
        {
            PrimarySoundAdapter = node;
            break;
        }
        if (!node)
            break;
    }
    return true;
}

extern "C" BOOL CALLBACK DSoundEnumCallback(LPGUID lpGuid, LPCSTR lpDescription, LPCSTR lpModule, LPVOID lpContext);

extern "C" bool DSoundEnumerate(LPVOID lpContext)
{
    if (DX_TRY(DirectSoundEnumerate(DSoundEnumCallback, lpContext)))
        return false;
    return true;
}

extern "C" BOOL CALLBACK DSoundEnumCallback(LPGUID lpGuid, LPCSTR lpDescription, LPCSTR lpModule, LPVOID lpContext)
{
    SOUND_ADAPTER_LIST *list = (SOUND_ADAPTER_LIST *)lpContext;
    SOUND_ADAPTER_NODE *node = new SOUND_ADAPTER_NODE(list->tail, NULL);

    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
    list->dwCount++;
    if (lpGuid)
    {
        node->body.adapterGuid = *lpGuid;
        node->body.lpAdapterGuid = &node->body.adapterGuid;
    }
    else
    {
        memset(&node->body.adapterGuid, 0, sizeof(GUID));
        node->body.lpAdapterGuid = NULL;
    }
    strcpy(node->body.description.lpString, lpDescription);
    strcpy(node->body.module.lpString, lpModule);
    return TRUE;
}

#pragma pack(push, 1)
typedef struct {
    void *PreferredDisplayAdapter;
    SOUND_ADAPTER_NODE *PreferredSoundAdapter;
    void *PreferredJoystick;
    void *VideoMode;
    int RenderMode;
    int WindowWidth;
    int WindowHeight;
    int AspectMode;
    bool PerspectiveCorrect;
    bool Dither;
    bool ZBuffer;
    bool BilinearFiltering;
    bool TripleBuffering;
    bool FullScreen;
    bool SoundEnabled;
    bool LaraMic;
    bool JoystickEnabled;
    bool Disable16BitTextures;
    bool DontSortPrimitives;
    bool FlipBroken;
    bool DisableFMV;
    int TexelAdjustMode;
    int NearestAdjustment;
    int LinearAdjustment;
} APP_SETTINGS_PART;
#pragma pack(pop)

enum InitResult {
    ERR_CantSetDSCooperativeLevel = 10
};

extern "C" {
extern APP_SETTINGS_PART SavedAppSettings;
extern SOUND_ADAPTER CurrentSoundAdapter;
extern int Var4d8e98;
extern HWND HGameWindow;
bool DSoundCreate(LPGUID lpGuid);
bool DSoundBufferTest(void);
}

extern "C" void WinSndStart(HWND hWnd)
{
    SOUND_ADAPTER *preferred;

    memset(SampleBuffers, 0, sizeof(SampleBuffers));
    memset(ChannelBuffers, 0, sizeof(ChannelBuffers));
    Var4d8e98 = 0;
    camera.mike_at_lara = SavedAppSettings.LaraMic;
    IsSoundEnabled = false;
    if (!SavedAppSettings.SoundEnabled || !SavedAppSettings.PreferredSoundAdapter)
        return;
    preferred = &SavedAppSettings.PreferredSoundAdapter->body;
    CurrentSoundAdapter = *preferred;
    if (!DSoundCreate(CurrentSoundAdapter.lpAdapterGuid))
        return;
    if (!hWnd)
        hWnd = HGameWindow;
    if (DX_TRY(DSound->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE)))
        throw ERR_CantSetDSCooperativeLevel;
    DSoundBufferTest();
    IsSoundEnabled = true;
}

extern "C" bool DSoundCreate(LPGUID lpGuid)
{
    if (DX_TRY(DirectSoundCreate(lpGuid, &DSound, NULL)))
        return false;
    return true;
}

extern "C" bool DSoundBufferTest(void)
{
    LPDIRECTSOUNDBUFFER dsBuffer;
    WAVEFORMATEX format;
    DSBUFFERDESC desc;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    if (DX_TRY(DSound->CreateSoundBuffer(&desc, &dsBuffer, NULL)))
        return false;
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 11025;
    format.nAvgBytesPerSec = 44100;
    format.nBlockAlign = 4;
    format.wBitsPerSample = 16;
    if (DX_TRY(dsBuffer->SetFormat(&format)))
        return false;
    if (dsBuffer) {
        dsBuffer->Release();
        dsBuffer = NULL;
    }
    return true;
}

extern "C" void WinSndFinish(void)
{
    WinSndFreeAllSamples();
    if (DSound) {
        DSound->Release();
        DSound = NULL;
    }
}

extern "C" bool WinSndIsSoundEnabled()
{
    return IsSoundEnabled;
}


extern "C" SOUND_ADAPTER_NODE *GetSoundAdapter(GUID *lpGuid)
{
    SOUND_ADAPTER_NODE *node;
    SOUND_ADAPTER_NODE *result;

    result = NULL;
    for (node = SoundAdapterList.head; node; node = node->next)
    {
        if (lpGuid && !memcmp(&node->body.adapterGuid, lpGuid, sizeof(GUID)))
        {
            result = node;
            break;
        }
        if (!node)
            break;
    }
    if (!result)
    {
        result = PrimarySoundAdapter;
        if (!result)
            result = NULL;
    }
    return result;
}

inline SOUND_ADAPTER_LIST::~SOUND_ADAPTER_LIST()
{
    SOUND_ADAPTER_NODE *node;
    SOUND_ADAPTER_NODE *nextNode;

    for (node = head; node; node = nextNode) {
        nextNode = node->next;
        delete node;
    }
    head = NULL;
    tail = NULL;
    dwCount = 0;
}

SOUND_ADAPTER_LIST SoundAdapterList;
SOUND_ADAPTER CurrentSoundAdapter;

#ifdef TR2_EXE
int SampleFreqs[256];
LPDIRECTSOUNDBUFFER SampleBuffers[256];
bool IsSoundEnabled;
LPDIRECTSOUND DSound;
int ChannelSamples[32];
LPDIRECTSOUNDBUFFER ChannelBuffers[32];
int Var4d8e98;
SOUND_ADAPTER_NODE *PrimarySoundAdapter;
#endif
