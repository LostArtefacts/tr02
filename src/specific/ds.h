#ifndef DS_H
#define DS_H

int DS_StartSample(int nSample, int nVolume, int nPitch, int nPan, DWORD dwFlags);
void DS_AdjustVolumeAndPan(int nHandle, int nVolume, int nPan);
void DS_AdjustPitch(int nHandle, int nPitch);
void DS_StopSample(int nHandle);
void DS_StopAllSamples();
bool DS_IsChannelPlaying(int nSlot);

#endif
