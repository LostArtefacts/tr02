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

void T_AddBackground(TEXTSTRING *textString, sint16 xsize, sint16 ysize, sint16 xoff, sint16 yoff, sint16 zoff, sint16 colour, SG_COL *gourptr, uint16 flags)
{
    uint32 scaleV, scaleH;

    scaleH = GetTextScaleH(textString->scaleH);
    scaleV = GetTextScaleV(textString->scaleV);

    if (textString)
    {
        textString->flags |= T_ADDBACKGROUND;
        textString->bgndSizeX = (sint16)((xsize * scaleH) >> 16);
        textString->bgndSizeY = (sint16)((ysize * scaleV) >> 16);
        textString->bgndOffX = (sint16)((xoff * scaleH) >> 16);
        textString->bgndOffY = (sint16)((yoff * scaleV) >> 16);
        textString->bgndOffZ = zoff;
        textString->bgndColour = colour;
        textString->bgndGour = gourptr;
        textString->bgndflags = flags;
    }
}

void T_RemoveBackground(TEXTSTRING *textString)
{
    if (textString)
        textString->flags &= ~T_ADDBACKGROUND;
}

void T_AddOutline(TEXTSTRING *textString, sint16 ok, sint16 colour, SG_COL *gourptr, uint16 flags)
{
    if (textString)
    {
        textString->flags |= T_ADDOUTLINE;
        textString->outlColour = colour;
        textString->outlGour = gourptr;
        textString->outlflags = flags;
    }
}

void T_RemoveOutline(TEXTSTRING *textString)
{
    if (textString)
        textString->flags &= ~T_ADDOUTLINE;
}

void T_CentreH(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;

    if (ok)
        textString->flags |= T_CENTRE_H;
    else
        textString->flags &= ~T_CENTRE_H;
}

void T_CentreV(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;

    if (ok)
        textString->flags |= T_CENTRE_V;
    else
        textString->flags &= ~T_CENTRE_V;
}

void T_RightAlign(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;

    if (ok)
        textString->flags |= T_RIGHTALIGN;
    else
        textString->flags &= ~T_RIGHTALIGN;
}

void T_BottomAlign(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;

    if (ok)
        textString->flags |= T_BOTTOMALIGN;
    else
        textString->flags &= ~T_BOTTOMALIGN;
}

uint8 Func4558d7(uint32 c);
uint8 Func4558e1(uint32 c);
uint32 Func455905(uint32 c);
uint32 Func455922(uint32 c);

sint32 T_GetTextWidth(TEXTSTRING *textString)
{
    uint32 scaleH;
    uint32 width;
    uint32 letter;
    uint8 *string;

    scaleH = GetTextScaleH(textString->scaleH);
    GetTextScaleV(textString->scaleV);
    width = 0;
    string = (uint8 *)textString->string;
    while (*string)
    {
        letter = *string++;
        if (Func4558d7(letter))
            continue;
        if (letter > 10 && letter < 32)
            continue;
        if (letter == ' ')
        {
            if (scaleH == 0x10000)
                width += textString->wordSpacing;
            else
                width += (textString->wordSpacing * scaleH) >> 16;
            continue;
        }
        if (letter >= 127 && Func4558e1(letter))
        {
            if (scaleH == 0x10000)
                width += 16;
            else
                width += (16 * scaleH) >> 16;
            continue;
        }
        if (letter < 11)
            letter += 81;
        else if (letter < 16)
            letter += 91;
        else
            letter = Func455905(letter);
        if (letter >= '0' && letter <= '9')
        {
            width += (12 * scaleH) >> 16;
            continue;
        }
        letter = Func455922(letter);
        if (scaleH == 0x10000)
            width += textString->letterSpacing + letter;
        else
            width += ((letter + textString->letterSpacing) * scaleH) >> 16;
    }
    return (width - textString->letterSpacing) & 0xfffe;
}

int T_RemovePrint(TEXTSTRING *textString)
{
    if (!textString)
        return 0;

    if (!(textString->flags & T_ACTIVE))
        return 0;

    textString->flags &= ~T_ACTIVE;
    T_numStrings--;
    return 1;
}

sint16 T_GetStringLen(char *string)
{
    sint16 len = 1;

    while (*string++)
    {
        len++;
        if (len > MAX_STRING_SIZE)
            return MAX_STRING_SIZE;
    }
    return len;
}

void T_DrawThisText(TEXTSTRING *textString);

void T_DrawText(void)
{
    int n;
    TEXTSTRING *textString = T_textStrings;

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        if (textString)
        {
            if (textString->flags & T_ACTIVE)
                T_DrawThisText(textString);
        }
        textString++;
    }
}

#define BORDERS 100

void S_DrawScreenSprite2d(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);

void draw_border(sint32 xpos, sint32 ypos, sint32 zpos, sint32 width, sint32 height)
{
    width -= 8;
    height -= 8;
    xpos += 4;
    ypos += 4;
    S_DrawScreenSprite2d(xpos, ypos, zpos, 0x10000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 0), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos + width, ypos, zpos, 0x10000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 1), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos + width, ypos + height, zpos, 0x10000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 2), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos, ypos + height, zpos, 0x10000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 3), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos, ypos, zpos, width * 0x2000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 4), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos + width, ypos, zpos, 0x10000, height * 0x2000, (sint16)(objects[BORDERS].mesh_index + 5), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos, ypos + height, zpos, width * 0x2000, 0x10000, (sint16)(objects[BORDERS].mesh_index + 6), 16 << 8, 0);
    S_DrawScreenSprite2d(xpos, ypos, zpos, 0x10000, height * 0x2000, (sint16)(objects[BORDERS].mesh_index + 7), 16 << 8, 0);
}

uint32 GetTextScaleH(uint32 scaleH)
{
    uint32 scale;
    int width;

    width = GetRenderWidth();
    if (width < 640)
        width = 640;
    scale = (width << 16) / 640;
    scaleH = (scaleH >> 8) * (scale >> 8);
    return scaleH;
}

uint32 GetTextScaleV(uint32 scaleV)
{
    uint32 scale;
    int height;

    height = GetRenderHeight();
    if (height < 480)
        height = 480;
    scale = (height << 16) / 480;
    return (scaleV >> 8) * (scale >> 8);
}
