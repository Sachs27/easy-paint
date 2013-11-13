#ifndef USER_PAINT_PANEL_H
#define USER_PAINT_PANEL_H


#include <sf/array.h>

#include "ui.h"
#include "canvas.h"
#include "brush.h"
#include "record.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"

struct resource_manager;


struct user_paint_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */
    struct canvas       canvas;
    struct record       record;

    struct ui_toolbox   toolbox;

    struct texture     *undo_image;
    struct ui_imagebox  undo;

    struct texture     *redo_image;
    struct ui_imagebox  redo;

    struct ui_imagebox  brush;

    struct ui_toolbox   brushbox;

    struct brush        brush_pen;
    struct texture     *brush_pen_image;
    struct ui_imagebox  brush_pen_icon;

    struct brush        brush_pencil;
    struct texture     *brush_pencil_image;
    struct ui_imagebox  brush_pencil_icon;

    struct brush        brush_eraser;
    struct texture     *brush_eraser_image;
    struct ui_imagebox  brush_eraser_icon;

    struct brush       *cur_brush;

    struct ui           blank;          /* blank ui for place holding */
};


struct user_paint_panel *user_paint_panel_create(int w, int h,
                                                 struct resource_manager *rm);


#endif /* USER_PAINT_PANEL_H */
