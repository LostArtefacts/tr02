#ifndef INVDATA_H
#define INVDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "text.h"

enum R_flags {
    R_CENTRE = 0,
    R_USE = (1 << 0),
    R_LEFTALIGN = (1 << 1),
    R_RIGHTALIGN = (1 << 2),
    R_HEADING = (1 << 3),
    R_BEST_TIME = (1 << 4),
    R_NORMAL_TIME = (1 << 5),
    R_NO_TIME = (1 << 6)
};

#define MAX_REQLINES 24

typedef struct request_info {
    uint16 noselector:1;
    uint16 items;
    uint16 selected;
    uint16 vis_lines;
    uint16 line_offset;
    uint16 line_oldoffset;
    uint16 pixwidth;
    uint16 line_height;
    sint16 xpos;
    sint16 ypos;
    sint16 zpos;
    sint16 itemtextlen;
    char *itemtexts1;
    char *itemtexts2;
    uint32 *itemtexts1_flags;
    uint32 *itemtexts2_flags;
    uint32 heading1_flags;
    uint32 heading2_flags;
    uint32 background_flags;
    uint32 moreup_flags;
    uint32 moredown_flags;
    uint32 texts1_flags[MAX_REQLINES];
    uint32 texts2_flags[MAX_REQLINES];
    TEXTSTRING *heading1text;
    TEXTSTRING *heading2text;
    TEXTSTRING *backgroundtext;
    TEXTSTRING *moreuptext;
    TEXTSTRING *moredowntext;
    TEXTSTRING *texts1[MAX_REQLINES];
    TEXTSTRING *texts2[MAX_REQLINES];
    char heading1_str[32];
    char heading2_str[32];
    int original_render_width;
    int original_render_height;
} REQUEST_INFO;

void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos);
int GetRenderHeight(void);
int GetRenderWidth(void);

extern REQUEST_INFO Load_Game_Requester;

void GetValidLevelsList(REQUEST_INFO *req);
void GetSavedGamesList(REQUEST_INFO *req);

extern uint32 RequesterFlags1[];
extern uint32 RequesterFlags2[];
extern uint32 SaveGameReqFlags1[];
extern uint32 SaveGameReqFlags2[];

void Init_Requester(REQUEST_INFO *req);
void RemoveAllReqItems(REQUEST_INFO *req);
void AddRequesterItem(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);
void ChangeRequesterItem(REQUEST_INFO *req, int item, char *text1, uint32 flags1, char *text2, uint32 flags2);

#ifdef __cplusplus
}
#endif

#endif
