#ifndef TITLE_H
#define TITLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum title_options {
    STARTGAME = 0,
    STARTSAVEDGAME = 1 << 8,
    STARTCINE = 2 << 8,
    STARTFMV = 3 << 8,
    STARTDEMO = 4 << 8,
    EXIT_TO_TITLE = 5 << 8,
    LEVELCOMPLETE = 6 << 8,
    EXITGAME = 7 << 8,
    EXIT_TO_OPTION = 8 << 8,
    TITLE_DESELECT = 9 << 8
} TITLE_OPTIONS;

extern int title_loaded;

sint32 LevelCompleteSequence(int level_number);

#ifdef __cplusplus
}
#endif

#endif
