#include "global.h"

extern sint16 layout[2][14];

extern "C" {
extern sint16 Option_Music_Volume;
extern sint16 Option_SoundFX_Volume;
extern int detail_level;
extern double game_sizer;
BOOL OpenGameRegistryKey(LPCTSTR key);
LONG CloseGameRegistryKey(void);
LONG SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value);
LONG SetRegistryFloatValue(LPCTSTR lpValueName, double value);
LONG SetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize);
bool GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue);
bool GetRegistryFloatValue(LPCTSTR lpValueName, double *value, double defaultValue);
bool GetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize, LPBYTE defaultValue);
void S_SoundSetMasterVolume(int volume);
void S_CDVolume(int volume);
}

extern "C" void S_LoadSettings(void)
{
    DWORD value;

    OpenGameRegistryKey("Game");
    GetRegistryDwordValue("MusicVolume", &value, 165);
    Option_Music_Volume = (sint16)value;
    GetRegistryDwordValue("SoundFXVolume", &value, 10);
    Option_SoundFX_Volume = (sint16)value;
    GetRegistryDwordValue("DetailLevel", (DWORD *)&detail_level, 1);
    GetRegistryFloatValue("Sizer", &game_sizer, 1.0);
    GetRegistryBinaryValue("Layout", (LPBYTE)layout[1], sizeof(layout[1]), NULL);
    CloseGameRegistryKey();
    S_SoundSetMasterVolume(6 * Option_SoundFX_Volume + 4);
    if (Option_Music_Volume)
        S_CDVolume(25 * Option_Music_Volume + 5);
    else
        S_CDVolume(0);
}
