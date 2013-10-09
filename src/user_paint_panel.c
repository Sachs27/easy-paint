#include <stddef.h>
#include <stdlib.h>

#include "app.h"
#include "user_paint_panel.h"


#define TOOLBOX_HEIGHT 48
#define TOOLBOX_MIN_WIDTH 288


/*
 * 下面的回调函数均使用了全局变量 g_app。
 */
static void undo_on_render(struct ui_imagebox *undo, struct renderer2d *r) {
    if (undo->image == NULL) {
        return;
    }

    if (canvas_record_canundo(g_app.upp->canvas)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, undo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           undo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static void undo_on_press(struct ui_imagebox *undo,
                          int n, int x[n], int y[n]) {
    canvas_record_undo(g_app.upp->canvas);
}

static void redo_on_render(struct ui_imagebox *redo, struct renderer2d *r) {
    if (redo->image == NULL) {
        return;
    }

    if (canvas_record_canredo(g_app.upp->canvas)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, redo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           redo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static void redo_on_press(struct ui_imagebox *redo,
                          int n, int x[n], int y[n]) {
    canvas_record_redo(g_app.upp->canvas);
}

static void user_paint_panel_on_push(struct user_paint_panel *upp,
                                     struct ui_manager *uim,
                                     int x, int y) {
    ui_manager_push(uim, x, y, (struct ui *) upp->canvas);

    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->undo);
    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->brush);
    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->redo);

    ui_manager_push(uim, x, y + upp->canvas->ui.area.h,
                    (struct ui *) upp->toolbox);
}

struct user_paint_panel *user_paint_panel_create(int w, int h) {
    struct user_paint_panel *upp;

    upp = malloc(sizeof(*upp));
    ui_init(&upp->ui, w, h);
    upp->brushes = sf_array_create(sizeof(struct brush), SF_ARRAY_NALLOC);
    sf_array_push(upp->brushes, brush_pencil_create());
    sf_array_push(upp->brushes, brush_pen_create());
    sf_array_push(upp->brushes, brush_eraser_create());
    upp->cur_brush_idx = 0;

    upp->canvas = canvas_create(w, h - TOOLBOX_HEIGHT);
    upp->canvas->cur_brush = SF_ARRAY_NTH(upp->brushes, upp->cur_brush_idx);

    upp->toolbox = ui_toolbox_create(w, TOOLBOX_HEIGHT,
                                     200, 200, 200, 200);

    upp->undo  = ui_imagebox_create(0, 0, texture_load_2d("res/icons/undo.png"));
    UI_CALLBACK(upp->undo, press, undo_on_press);
    UI_CALLBACK(upp->undo, render, undo_on_render);

    upp->redo  = ui_imagebox_create(0, 0, texture_load_2d("res/icons/redo.png"));
    UI_CALLBACK(upp->redo, press, redo_on_press);
    UI_CALLBACK(upp->redo, render, redo_on_render);

    upp->brush = ui_imagebox_create(0, 0, upp->canvas->cur_brush->icon);

    UI_CALLBACK(upp, push, user_paint_panel_on_push);

    return upp;
}

void user_paint_panel_resize(struct user_paint_panel *upp, int w, int h) {
    canvas_resize(upp->canvas, w, h - TOOLBOX_HEIGHT);

    if (w < TOOLBOX_MIN_WIDTH) {
        ui_toolbox_resize(upp->toolbox, TOOLBOX_MIN_WIDTH, upp->toolbox->ui.area.h);
    } else {
        ui_toolbox_resize(upp->toolbox, w, upp->toolbox->ui.area.h);
    }
    ui_toolbox_move(upp->toolbox, upp->canvas->ui.area.x,
                    upp->canvas->ui.area.y + upp->canvas->ui.area.h);
}
