#ifndef CINEMA_H
#define CINEMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

extern sint16 *cine;
extern sint16 cine_loaded;
extern sint16 num_cine_frames;
extern int cutscene_track;
void SetCutsceneTrack(int track);

#ifdef __cplusplus
}
#endif

#endif
