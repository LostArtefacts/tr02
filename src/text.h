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

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string);
void T_ChangeText(TEXTSTRING *textString, char *string);
void T_AddBackground(TEXTSTRING *textString, sint16 xsize, sint16 ysize, sint16 xoff, sint16 yoff, sint16 zoff, sint16 colour, SG_COL *gourptr, uint16 flags);
void T_RemoveBackground(TEXTSTRING *textString);
void T_AddOutline(TEXTSTRING *textString, sint16 ok, sint16 colour, SG_COL *gourptr, uint16 flags);
void T_RemoveOutline(TEXTSTRING *textString);
void T_CentreH(TEXTSTRING *textString, sint16 ok);
void T_CentreV(TEXTSTRING *textString, sint16 ok);
int T_RemovePrint(TEXTSTRING *tString);

#ifdef __cplusplus
}
#endif

#endif
