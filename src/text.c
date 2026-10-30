#include "game.h"
#include "text.h"

#define MAX_TEXT_STRINGS 64
#define MAX_STRING_SIZE 64

#define T_ACTIVE (1 << 0)
#define T_FLASH (1 << 1)
#define T_ROTATE_H (1 << 2)
#define T_ROTATE_V (1 << 3)
#define T_CENTRE_H (1 << 4)
#define T_CENTRE_V (1 << 5)
#define T_RIGHTALIGN (1 << 7)
#define T_BOTTOMALIGN (1 << 8)
#define T_ADDBACKGROUND (1 << 9)
#define T_ADDOUTLINE (1 << 10)
#define T_RIGHTJUSTIFY (1 << 11)

extern TEXTSTRING T_textStrings[MAX_TEXT_STRINGS];
extern sint16 T_numStrings;

void DisplayModeInfo(char *string);

void T_InitPrint(void)
{
    int n;
    TEXTSTRING *textString = T_textStrings;

    DisplayModeInfo(0);

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        textString->flags = 0;
        textString++;
    }
    T_numStrings = 0;
}

extern char T_theStrings[MAX_TEXT_STRINGS * MAX_STRING_SIZE];

sint16 T_GetStringLen(char *string);
uint32 GetTextScaleH(uint32 scaleH);
uint32 GetTextScaleV(uint32 scaleV);

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string)
{
    uint32 scaleV, scaleH;
    int n, length;
    TEXTSTRING *textString = T_textStrings;

    if (string == NULL)
        return 0;

    if (T_numStrings >= MAX_TEXT_STRINGS)
        return 0;

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        if (!(textString->flags & T_ACTIVE))
            break;
        textString++;
    }

    if (n >= MAX_TEXT_STRINGS)
        return 0;

    length = T_GetStringLen(string);
    if (length >= MAX_STRING_SIZE)
        length = MAX_STRING_SIZE - 1;

    textString->string = &T_theStrings[n * MAX_STRING_SIZE];
    textString->scaleH = 0x10000;
    textString->scaleV = 0x10000;
    scaleH = GetTextScaleH(0x10000);
    scaleV = GetTextScaleV(0x10000);
    textString->xpos = (sint16)((xpos * scaleH) >> 16);
    textString->ypos = (sint16)((ypos * scaleV) >> 16);
    textString->zpos = (sint16)zpos;
    textString->letterSpacing = 1;
    textString->wordSpacing = 6;
    memcpy(textString->string, string, length + 1);
    textString->flags = T_ACTIVE;
    textString->textflags = 0;
    textString->outlflags = 0;
    textString->bgndflags = 0;
    textString->bgndSizeX = 0;
    textString->bgndSizeY = 0;
    textString->bgndOffX = 0;
    textString->bgndOffY = 0;
    textString->bgndOffZ = 0;
    T_numStrings++;
    return textString;
}

void T_ChangeText(TEXTSTRING *textString, char *string)
{
    if (string == NULL || textString == NULL)
        return;

    if (textString->flags & T_ACTIVE)
    {
        if (T_GetStringLen(string) > MAX_STRING_SIZE)
            string[MAX_STRING_SIZE - 1] = 0;
        memcpy(textString->string, string, MAX_STRING_SIZE);
    }
}

void T_SetScale(TEXTSTRING *textString, sint32 scaleH, sint32 scaleV)
{
    if (textString)
    {
        textString->scaleH = scaleH;
        textString->scaleV = scaleV;
    }
}

void T_FlashText(TEXTSTRING *textString, sint16 ok, sint16 rate)
{
    if (textString == NULL)
        return;

    if (ok)
    {
        textString->flags |= T_FLASH;
        textString->flashRate = rate;
        textString->flashCount = rate;
    }
    else
        textString->flags &= ~T_FLASH;
}
