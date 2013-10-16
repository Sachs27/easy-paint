#include <stddef.h>
#include <stdlib.h>

#include "app.h"
#include "user_paint_panel.h"
#include "record.h"
#include "canvas.h"
#include "brush.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"


#define TOOLBOX_HEIGHT 48
#define TOOLBOX_MIN_WIDTH 288


/*
 * The following callback function use global variable g_app.
 */
static void undo_on_render(struct ui_imagebox *undo, struct renderer2d *r) {
    struct user_paint_panel *upp = g_app.upp;

    if (undo->image == NULL) {
        return;
    }
    if (record_canundo(upp->record)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, undo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           undo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static void undo_on_press(struct ui_imagebox *undo,
                          int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;

    if (upp->brushbox->ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) upp->brushbox);
        ui_hide(upp->blank);
    }

    record_undo(upp->record, upp->canvas);
}

static void redo_on_render(struct ui_imagebox *redo, struct renderer2d *r) {
    struct user_paint_panel *upp = g_app.upp;

    if (redo->image == NULL) {
        return;
    }

    if (record_canredo(upp->record)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, redo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           redo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static void redo_on_press(struct ui_imagebox *redo,
                          int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;

    if (upp->brushbox->ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) upp->brushbox);
        ui_hide(upp->blank);
    }

    record_redo(upp->record, upp->canvas);
}

static void brush_on_press(struct ui_imagebox *brush,
                           int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;

    if (upp->brushbox->ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) g_app.upp->brushbox);
        ui_hide(upp->blank);
    } else {
        ui_show((struct ui *) g_app.upp->brushbox);
        ui_show(upp->blank);
    }
}

static void brushicon_on_press(struct ui_imagebox *button,
                               int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;
    int i = 0;

    SF_ARRAY_BEGIN(upp->brushicons, struct ui_imagebox *, p);
        if (button == *p) {
            break;
        }
        ++i;
    SF_ARRAY_END();

    assert(i < upp->brushicons->nelts);

    upp->cur_brush = SF_ARRAY_NTH(upp->brushes, i);
    ui_imagebox_set_image(upp->brush, upp->cur_brush->icon);

    ui_hide((struct ui *) upp->brushbox);
    ui_hide(upp->blank);
}


static void blank_on_press(struct ui *ui, int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;

    ui_hide((struct ui *) upp->brushbox);
    ui_hide(upp->blank);
}

static void user_paint_panel_on_push(struct user_paint_panel *upp,
                                     struct ui_manager *uim,
                                     int x, int y) {
    ui_manager_push(uim, x, y, (struct ui *) upp->canvas);

    ui_manager_push(uim, x, y, upp->blank);

    SF_ARRAY_BEGIN(upp->brushicons, struct ui_imagebox *, p);
        struct ui_imagebox *button = *p;
        ui_toolbox_add_button(upp->brushbox, (struct ui *) button);
    SF_ARRAY_END();

    ui_manager_push(uim, x, y + upp->canvas->ui.area.h - 2 * TOOLBOX_HEIGHT,
                    (struct ui *) upp->brushbox);

    ui_hide((struct ui *) upp->brushbox);
    ui_hide(upp->blank);

    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->undo);
    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->brush);
    ui_toolbox_add_button(upp->toolbox, (struct ui *) upp->redo);

    ui_manager_push(uim, x, y + upp->canvas->ui.area.h - TOOLBOX_HEIGHT,
                    (struct ui *) upp->toolbox);
}

static int canvas_lastx;
static int canvas_lasty;

static void canvas_on_press(struct canvas *canvas,
                            int n, int x[n], int y[n]) {
    struct user_paint_panel *upp = g_app.upp;

    if (n == 1) {
        canvas_screen_to_canvas(canvas, x[0] + canvas->ui.area.x,
                                y[0] + canvas->ui.area.y,
                                &canvas_lastx, &canvas_lasty);
        record_begin(upp->record, canvas);
    }
}

static void canvas_on_release(struct canvas *canvas) {
    record_end(canvas->record);
}

static void canvas_on_update(struct canvas *canvas, struct input_manager *im,
                             double dt) {
    struct user_paint_panel *upp = g_app.upp;

    if (canvas->record) {
        int mx, my;

        canvas_screen_to_canvas(canvas, im->mouse.x, im->mouse.y, &mx, &my);

        if (sf_rect_iscontain(&canvas->viewport, mx, my)
            && (mx != canvas_lastx || my != canvas_lasty)) {
            brush_drawline(upp->cur_brush, canvas,
                           canvas_lastx, canvas_lasty, mx, my);
        }

        canvas_lastx = mx;
        canvas_lasty = my;
    }
}


struct user_paint_panel *user_paint_panel_create(int w, int h) {
    struct user_paint_panel *upp;

    upp = malloc(sizeof(*upp));
    ui_init(&upp->ui, w, h);

    upp->brushes = sf_array_create(sizeof(struct brush), SF_ARRAY_NALLOC);
    sf_array_push(upp->brushes, brush_pencil_create());
    sf_array_push(upp->brushes, brush_pen_create());
    sf_array_push(upp->brushes, brush_eraser_create());

    upp->brushicons = sf_array_create(sizeof(struct ui_imagebox *),
                                      upp->brushes->nalloc);

    SF_ARRAY_BEGIN(upp->brushes, struct brush, b);
        struct ui_imagebox *ib = ui_imagebox_create(0, 0, b->icon);
        UI_CALLBACK(ib, press, brushicon_on_press);
        sf_array_push(upp->brushicons, &ib);
    SF_ARRAY_END();

    upp->canvas = canvas_create(w, h);
    UI_CALLBACK(upp->canvas, update, canvas_on_update);
    UI_CALLBACK(upp->canvas, press, canvas_on_press);
    UI_CALLBACK(upp->canvas, release, canvas_on_release);
    upp->cur_brush = SF_ARRAY_NTH(upp->brushes, 0);

    upp->record = record_create();

    upp->toolbox = ui_toolbox_create(w, TOOLBOX_HEIGHT,
                                     128, 128, 128, 240);

    upp->brushbox = ui_toolbox_create(w, TOOLBOX_HEIGHT,
                                      222, 222, 222, 240);

    upp->undo  = ui_imagebox_create(0, 0, texture_load_2d("res/icons/undo.png"));
    UI_CALLBACK(upp->undo, press, undo_on_press);
    UI_CALLBACK(upp->undo, render, undo_on_render);

    upp->redo  = ui_imagebox_create(0, 0, texture_load_2d("res/icons/redo.png"));
    UI_CALLBACK(upp->redo, press, redo_on_press);
    UI_CALLBACK(upp->redo, render, redo_on_render);

    upp->brush = ui_imagebox_create(0, 0, upp->cur_brush->icon);
    UI_CALLBACK(upp->brush, press, brush_on_press);

    upp->blank = malloc(sizeof(struct ui));
    ui_init(upp->blank, w, h - 2 * TOOLBOX_HEIGHT);
    UI_CALLBACK(upp->blank, press, blank_on_press);

    UI_CALLBACK(upp, push, user_paint_panel_on_push);

    return upp;
}

void user_paint_panel_resize(struct user_paint_panel *upp, int w, int h) {
    canvas_resize(upp->canvas, w, h - TOOLBOX_HEIGHT);

    if (w < TOOLBOX_MIN_WIDTH) {
        ui_toolbox_resize(upp->toolbox,
                          TOOLBOX_MIN_WIDTH, upp->toolbox->ui.area.h);
        ui_toolbox_resize(upp->brushbox,
                          TOOLBOX_MIN_WIDTH, upp->toolbox->ui.area.h);
    } else {
        ui_toolbox_resize(upp->toolbox, w, upp->toolbox->ui.area.h);
        ui_toolbox_resize(upp->brushbox, w, upp->toolbox->ui.area.h);
    }

    ui_toolbox_move(upp->toolbox, upp->canvas->ui.area.x,
                    upp->canvas->ui.area.y + upp->canvas->ui.area.h);

    ui_toolbox_move(upp->brushbox, upp->canvas->ui.area.x,
                    upp->canvas->ui.area.y + upp->canvas->ui.area.h
                    - TOOLBOX_HEIGHT);
}
