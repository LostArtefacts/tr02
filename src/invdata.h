#ifndef INVDATA_H
#define INVDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "text.h"
#include "3dglodef.h"
#include "input.h"

#define DESELECT_ITEM IN_DESELECT
#define SELECT_ITEM IN_SELECT

#define DEC_HBAR IN_LEFT
#define INC_HBAR IN_RIGHT
#define PREV_HBAR IN_FORWARD
#define NEXT_HBAR IN_BACK

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

typedef struct inventory_sprite {
    sint16 shape;
    sint16 x;
    sint16 y;
    sint16 z;
    sint32 param1;
    sint32 param2;
    SG_COL *grdptr;
    sint16 sprnum;
} INVENTORY_SPRITE;

typedef struct inventory_item {
    char *itemText;
    sint16 object_number;
    sint16 frames_total;
    sint16 current_frame;
    sint16 goal_frame;
    sint16 open_frame;
    sint16 anim_direction;
    sint16 anim_speed;
    sint16 anim_count;
    PHD_ANGLE pt_xrot_sel;
    PHD_ANGLE pt_xrot;
    PHD_ANGLE x_rot_sel;
    PHD_ANGLE x_rot_nosel;
    PHD_ANGLE x_rot;
    PHD_ANGLE y_rot_sel;
    PHD_ANGLE y_rot;
    sint32 ytrans_sel;
    sint32 ytrans;
    sint32 ztrans_sel;
    sint32 ztrans;
    uint32 which_meshes;
    uint32 drawn_meshes;
    sint16 inv_pos;
    INVENTORY_SPRITE **sprlist;
    sint32 misc_data[4];
} INVENTORY_ITEM;

extern sint32 inputDB;

extern INVENTORY_SPRITE is_sound_muslevel[];
extern INVENTORY_SPRITE is_sound_sfxlevel[];
extern INVENTORY_SPRITE is_sound_muslevellow[];
extern INVENTORY_SPRITE is_sound_sfxlevellow[];

void ShowStatsText(char *time_str, int type);
void ShowGymStatsText(char *time_str, int type);
void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos);
int GetRenderHeight(void);
int GetRenderWidth(void);
void do_compass_option(INVENTORY_ITEM *inv_item);
void do_detail_option(INVENTORY_ITEM *inv_item);
void do_sound_option(INVENTORY_ITEM *inv_item);
void S_RemoveCtrlText(void);

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
