#ifndef FRONTEND_H
#define FRONTEND_H

#ifdef __cplusplus
extern "C" {
#endif

void S_FinishInventory(void);
int S_PlayFMV(char *fname, int sequence);
int S_IntroFMV(char *movie1, char *movie2);

#ifdef __cplusplus
}
#endif

#endif
