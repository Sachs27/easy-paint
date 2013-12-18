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
#define RES_TEXTURE_ICON_NEW  "assets/icons/new.png"
#define RES_TEXTURE_ICON_SAVE "assets/icons/save.png"
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
#define RESOURCE_NTEXTURES 16
#define RESOURCE_NRECORDS 0


int rm_init(const char *res_path, const char *save_path);

void rm_term(void);

struct texture *rm_load_texture(const char *filename);

struct record *rm_load_record(const char *filename);

int rm_set_record_size(int w, int h);

/*
 * save the LastRecord.epr, if there is no one then will
 * create a empty record
 */
int rm_save_last_record(void);

struct record *rm_load_last_record(void);

/*
 * save the record r to the previous opened file
 */
int rm_save_user_define_record(struct record *r);

/*
 * save the record to another file
 */
int rm_save_as_user_define_record(struct record *r, const char *filename);

struct record *rm_load_user_define_record(const char *filename);

size_t rm_get_user_define_record_count(void);

/* there are at least one record (LastRecord.epr) */
size_t rm_get_all_user_define_records(struct record **records, size_t len);

int rm_del_user_define_record(const char *filename);

int rm_del_last_record(void);


#endif /* RESMGR_H */
