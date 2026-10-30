#ifndef WINMAIN_H
#define WINMAIN_H

#define RM_Software 1
#define RM_Hardware 2

typedef struct {
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    void *VideoMode;
    int RenderMode;
    int WindowWidth;
    int WindowHeight;
    int AspectMode;
    bool PerspectiveCorrect;
    bool Dither;
    bool ZBuffer;
} APP_SETTINGS;

#ifdef __cplusplus
extern "C" {
#endif

extern APP_SETTINGS SavedAppSettings;

#ifdef __cplusplus
}
#endif

#endif
