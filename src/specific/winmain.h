#ifndef WINMAIN_H
#define WINMAIN_H

#define RM_Hardware 2

typedef struct {
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    void *VideoMode;
    int RenderMode;
} APP_SETTINGS;

#ifdef __cplusplus
extern "C" {
#endif

extern APP_SETTINGS SavedAppSettings;

#ifdef __cplusplus
}
#endif

#endif
