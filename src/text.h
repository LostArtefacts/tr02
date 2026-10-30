#ifndef TEXT_H
#define TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "inventry.h"

typedef struct textstringinfo {
    uint32 flags;
    uint16 textflags;
    uint16 bgndflags;
    uint16 outlflags;
    sint16 xpos;
    sint16 ypos;
    sint16 zpos;
    sint16 letterSpacing;
    sint16 wordSpacing;
    sint16 flashRate;
    sint16 flashCount;
    sint16 bgndColour;
    SG_COL *bgndGour;
    sint16 outlColour;
    SG_COL *outlGour;
    sint16 bgndSizeX;
    sint16 bgndSizeY;
    sint16 bgndOffX;
    sint16 bgndOffY;
    sint16 bgndOffZ;
    sint32 scaleH;
    sint32 scaleV;
    char *string;
} TEXTSTRING;

#ifdef __cplusplus
}
#endif

#endif
