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

void S_DrawPickup(int sx, int sy, int scale, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, SG_COL *grdptr, uint16 flags);

#define SECRET_ITEM1 190
#define ALPHABET 255

void T_DrawThisText(TEXTSTRING *textString)
{
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 scaleH;
    sint32 scaleV;
    sint32 textWidth;
    sint32 boxX;
    sint32 boxY;
    sint32 boxW;
    sint32 boxH;
    sint32 halfW;
    sint32 halfH;
    sint32 textHeight;
    uint32 letter;
    uint32 sprite;
    uint8 *string;

    boxW = 0;
    boxH = 0;
    scaleH = GetTextScaleH(textString->scaleH);
    scaleV = GetTextScaleV(textString->scaleV);
    textHeight = (uint32)(11 * scaleV) >> 16;
    if (textString->flags & T_FLASH)
    {
        textString->flashCount -= (sint16)camera.number_frames;
        if (textString->flashCount <= -textString->flashRate)
            textString->flashCount = textString->flashRate;
        else if (textString->flashCount < 0)
            return;
    }
    x = textString->xpos;
    string = (uint8 *)textString->string;
    y = textString->ypos;
    z = textString->zpos;
    textWidth = T_GetTextWidth(textString);
    if (textString->flags & T_CENTRE_H)
        x += (GetRenderWidth() - textWidth) / 2;
    else if (textString->flags & T_RIGHTALIGN)
        x += GetRenderWidth() - textWidth;
    if (textString->flags & T_CENTRE_V)
        y += GetRenderHeight() / 2;
    else if (textString->flags & T_BOTTOMALIGN)
        y += GetRenderHeight();
    boxX = x + textString->bgndOffX - ((uint32)(2 * scaleH) >> 16);
    boxY = y + textString->bgndOffY - ((uint32)(4 * scaleV) >> 16) - textHeight;
    while (*string)
    {
        letter = *string++;
        if (letter > 18 && letter < 32)
            continue;
        if (letter == ' ')
        {
            x += (uint32)(textString->wordSpacing * scaleH) >> 16;
            continue;
        }
        if (letter >= 127)
        {
            if (letter <= 129)
            {
                S_DrawPickup(x + 10, y, 7144, objects[letter + SECRET_ITEM1 - 127].mesh_index, 4096, 0);
                x += (uint32)(16 * scaleH) >> 16;
                continue;
            }
            if (Func4558e1(letter))
                continue;
        }
        if (letter < 11)
            sprite = letter + 81;
        else if (letter <= 18)
            sprite = letter + 91;
        else
            sprite = Func455905(letter);
        if (letter >= '0' && letter <= '9')
            x += (uint32)((12 - (sint32)Func455922(sprite)) / 2 * scaleH) >> 16;
        if (x > 0 && x < GetRenderWidth() && y > 0 && y < GetRenderHeight())
            S_DrawScreenSprite2d(x, y, z, scaleH, scaleV, (sint16)(objects[ALPHABET].mesh_index + sprite), 4096, textString->textflags);
        if (letter >= '0' && letter <= '9')
            x += (uint32)((12 - (12 - (sint32)Func455922(sprite)) / 2) * scaleH) >> 16;
        else if (scaleH == 0x10000)
            x += textString->letterSpacing + Func455922(sprite);
        else
            x += (uint32)((Func455922(sprite) + textString->letterSpacing) * scaleH) >> 16;
    }
    if ((textString->flags & T_ADDBACKGROUND) || (textString->flags & T_ADDOUTLINE))
    {
        if (textString->bgndSizeX)
        {
            boxX += textWidth / 2;
            textWidth = textString->bgndSizeX;
            boxX -= textWidth / 2;
        }
        boxW = textWidth + 4;
        if (textString->bgndSizeY)
            boxH = textString->bgndSizeY;
        else
            boxH = (uint32)(16 * scaleV) >> 16;
    }
    if (textString->flags & T_ADDBACKGROUND)
    {
        if (textString->bgndGour)
        {
            halfW = boxW / 2;
            halfH = boxH / 2;
            S_DrawScreenFBox(boxX, boxY, textString->bgndOffZ + z + 2, boxW, boxH, textString->bgndColour, textString->bgndGour, textString->bgndflags);
        }
        else
        {
            S_DrawScreenFBox(boxX, boxY, textString->bgndOffZ + z + 2, boxW, boxH, textString->bgndColour, 0, textString->bgndflags);
        }
    }
    if (textString->flags & T_ADDOUTLINE)
        draw_border(boxX, boxY, z, boxW, boxH);
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

#ifdef TR2_EXE
TEXTSTRING T_textStrings[MAX_TEXT_STRINGS];
sint16 T_numStrings;
char T_theStrings[MAX_TEXT_STRINGS * MAX_STRING_SIZE];
#endif
