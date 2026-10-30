#ifndef FILE_H
#define FILE_H

#ifdef __cplusplus
extern "C" {
#endif

int S_LoadLevelFile(char *fname, int level_number, int type);
void S_UnloadLevelFile(void);
void S_AdjustTexelCoordinates(void);
int S_FrontEndCheck(void *data, int size);
int S_LoadGame(void *data, int size, int slot);
int S_SaveGame(void *data, int size, int slot);

#ifdef __cplusplus
}
#endif

#endif
