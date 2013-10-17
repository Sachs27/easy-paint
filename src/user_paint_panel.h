#ifndef USER_PAINT_PANEL_H
#define USER_PAINT_PANEL_H


#include <sf_array.h>

#include "ui.h"

struct canvas;
struct brush;
struct record;
struct ui_toolbox;
struct ui_imagebox;


struct user_paint_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */
    struct canvas      *canvas;
    struct brush       *cur_brush;

    struct record      *record;

    struct ui_toolbox  *toolbox;
    struct ui_imagebox *undo;
    struct ui_imagebox *redo;
    struct ui_imagebox *brush;

    struct ui_toolbox  *brushbox;
    struct sf_array    *brushicons;     /* elt: (struct ui_imagebox *) */
    struct sf_array    *brushes;        /* elt: (struct brush) */

    struct ui          *blank;          /* blank ui for place holding */
};


struct user_paint_panel *user_paint_panel_create(int w, int h);

void user_paint_panel_resize(struct user_paint_panel *upp, int w, int h);


#endif /* USER_PAINT_PANEL_H */
