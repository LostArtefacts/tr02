#ifndef FILE_H
#define FILE_H

#ifdef __cplusplus
extern "C" {
#endif

int S_LoadLevelFile(char *fname, int level_number, int type);
void S_UnloadLevelFile(void);
void S_AdjustTexelCoordinates(void);

#ifdef __cplusplus
}
#endif

#endif
