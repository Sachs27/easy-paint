#ifndef USER_PAINT_PANEL_H
#define USER_PAINT_PANEL_H


#include <sf/array.h>

#include "ui.h"
#include "canvas.h"
#include "brush.h"
#include "record.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"
#include "ui_color_picker.h"
#include "ui_replay_panel.h"

struct resource_manager;


struct user_paint_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */
    struct canvas           canvas;
    struct record          *record;
    struct record          *replay_record;
    int8_t                  isplaying;
    int8_t                  isresizing;
    int8_t                  isfirstframe;
    int8_t                  isadject;
    int                     cansave;
    struct ui_replay_panel  urp;
    struct ui_replay_panel  mini_urp;
    struct ui               border;

    int                     lastx;
    int                     lasty;

    struct ui_color_picker  color_picker;
    struct ui_toolbox       toolbox;

    struct texture         *undo_image;
    struct ui_imagebox      undo;

    struct texture         *redo_image;
    struct ui_imagebox      redo;

    struct ui_imagebox      brush;

    struct texture         *replay_image;
    struct ui_imagebox      replay;

    struct texture         *save_image;
    struct ui_imagebox      save;

    struct ui_toolbox       brushbox;

    struct brush            brush_pen;
    struct texture         *brush_pen_image;
    struct ui_imagebox      brush_pen_icon;

    struct brush            brush_pencil;
    struct texture         *brush_pencil_image;
    struct ui_imagebox      brush_pencil_icon;

    struct brush            brush_eraser;
    struct texture         *brush_eraser_image;
    struct ui_imagebox      brush_eraser_icon;

    struct brush           *cur_brush;

    struct ui               blank;          /* blank ui for place holding */
};


int user_paint_panel_init(struct user_paint_panel *upp, int w, int h);

void user_paint_panel_set_record(struct user_paint_panel *upp,
                                 struct record *r);

void user_paint_panel_set_replay_record(struct user_paint_panel *upp,
                                        struct record *r);


#endif /* USER_PAINT_PANEL_H */
