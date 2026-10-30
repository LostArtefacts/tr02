#ifndef FRONTEND_H
#define FRONTEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../inventry.h"

void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_FinishInventory(void);
void S_FadeToBlack(void);
void S_Wait(int nframes, int anykey);
int S_PlayFMV(char *fname, int sequence);
int S_IntroFMV(char *movie1, char *movie2);
SG_COL S_COLOUR(int R, int G, int B);

#ifdef __cplusplus
}
#endif

#endif
