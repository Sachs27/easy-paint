#ifndef RESMGR_H
#define RESMGR_H


#include <sf/list.h>

#include "filesystem.h"

struct texture;

struct record;

#define RES_TEXTURE_ICON_PEN "assets/icons/pen.png"
#define RES_TEXTURE_ICON_PENCIL "assets/icons/pencil.png"
#define RES_TEXTURE_ICON_ERASER "assets/icons/eraser.png"
#define RES_TEXTURE_ICON_UNDO "assets/icons/undo.png"
#define RES_TEXTURE_ICON_REDO "assets/icons/redo.png"
#define RES_TEXTURE_ICON_STOP "assets/icons/stop.png"
#define RES_TEXTURE_ICON_PAUSE "assets/icons/pause.png"
#define RES_TEXTURE_ICON_PLAY "assets/icons/play.png"
#define RES_TEXTURE_ICON_FASTFORWARD "assets/icons/fastforward.png"
#define RES_TEXTURE_ICON_REWIND "assets/icons/rewind.png"
#define RES_TEXTURE_ICON_PARENT "assets/icons/parent.png"
#define RES_TEXTURE_ICON_LOGO "assets/icons/logo.png"
#define RES_TEXTURE_ICON_LABEL1 "assets/icons/label1.png"
#define RES_TEXTURE_ICON_LABEL2 "assets/icons/label2.png"
#define RES_TEXTURE_ICON_LABEL3 "assets/icons/label3.png"
#define RESOURCE_NTEXTURES 15
#define RESOURCE_NRECORDS 0


int rm_init(const char *res_path, const char *save_path);

void rm_term(void);

struct texture *rm_load_texture(const char *filename);

struct record *rm_load_record(const char *filename);

struct record *rm_load_last_record(void);

struct record *rm_load_user_define_record(const char *filename);

int rm_save_last_record(void);

int rm_save_user_define_record(const char *filename, struct record *r);


#endif /* RESMGR_H */
