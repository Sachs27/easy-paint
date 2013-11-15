#include <assert.h>
#include <stddef.h>

#include <sf/utils.h>

#include "user_paint_panel.h"
#include "system.h"
#include "resource_manager.h"


#define TOOLBOX_HEIGHT 48
#define TOOLBOX_MIN_WIDTH 288


/*
 * The following callback function use global variable g_app.
 */
static void undo_on_render(struct ui *ui, struct renderer2d *r) {
    struct ui_imagebox *undo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(undo, struct user_paint_panel, undo);

    if (undo->image == NULL) {
        return;
    }
    if (record_canundo(&upp->record)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, undo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           undo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static int undo_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *undo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(undo, struct user_paint_panel, undo);

    if (upp->brushbox.ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) &upp->brushbox);
        ui_hide(&upp->blank);
    }

    record_undo(&upp->record, &upp->canvas);
    return 0;
}

static void redo_on_render(struct ui *ui, struct renderer2d *r) {
    struct ui_imagebox *redo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(redo, struct user_paint_panel, redo);

    if (redo->image == NULL) {
        return;
    }

    if (record_canredo(&upp->record)) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, redo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(r, 0, 0, 0, 0,
                                           redo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static int redo_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *redo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(redo, struct user_paint_panel, redo);

    if (upp->brushbox.ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) &upp->brushbox);
        ui_hide(&upp->blank);
    }

    record_redo(&upp->record, &upp->canvas);
    return 0;
}

static int brush_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *brush = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(brush, struct user_paint_panel, brush);

    if (upp->brushbox.ui.state == UI_STATE_SHOW) {
        ui_hide((struct ui *) &upp->brushbox);
        ui_hide(&upp->blank);
    } else {
        ui_show((struct ui *) &upp->brushbox);
        ui_show(&upp->blank);
    }
    return 0;
}
#if 0
static void brushicon_on_press(struct ui_imagebox *button, int x, int y) {
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
    ui_imagebox_set_image(&upp->brush, upp->cur_brush->icon);

    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
}
#endif

static int brush_pen_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *brush_pen_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_pen_icon,
                                                   struct user_paint_panel,
                                                   brush_pen_icon);
    upp->cur_brush = &upp->brush_pen;
    ui_imagebox_set_image(&upp->brush, upp->brush_pen_icon.image);
    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
    return 0;
}

static int brush_pencil_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *brush_pencil_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_pencil_icon,
                                                   struct user_paint_panel,
                                                   brush_pencil_icon);
    upp->cur_brush = &upp->brush_pencil;
    ui_imagebox_set_image(&upp->brush, upp->brush_pencil_icon.image);
    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
    return 0;
}

static int brush_eraser_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *brush_eraser_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_eraser_icon,
                                                   struct user_paint_panel,
                                                   brush_eraser_icon);
    upp->cur_brush = &upp->brush_eraser;
    ui_imagebox_set_image(&upp->brush, upp->brush_eraser_icon.image);
    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
    return 0;
}

static int blank_on_press(struct ui *blank, int x, int y) {
    struct user_paint_panel *upp =
        sf_container_of(blank, struct user_paint_panel, blank);

    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
    return 0;
}

static int canvas_lastx;
static int canvas_lasty;
static int isdrawing = 0;

static int canvas_on_press(struct ui *ui, int x, int y) {
    struct canvas *canvas = (struct canvas *) ui;
    struct user_paint_panel *upp =
        sf_container_of(canvas, struct user_paint_panel, canvas);
    int xscreen, yscreen;

    ui_get_screen_pos((struct ui *) canvas, &xscreen, &yscreen);
    canvas_screen_to_canvas(canvas, x + xscreen, y + yscreen,
                            &canvas_lastx, &canvas_lasty);
    isdrawing = 1;
    record_begin(&upp->record, canvas);

    return 0;
}

static void canvas_on_release(struct ui *ui) {
    struct canvas *canvas = (struct canvas *) ui;
    isdrawing = 0;
    record_end(canvas->record);
}

static void canvas_on_update(struct ui *ui, struct input_manager *im,
                             double dt) {
    struct canvas *canvas = (struct canvas *) ui;
    struct user_paint_panel *upp =
        sf_container_of(canvas, struct user_paint_panel, canvas);

    if (isdrawing) {
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

    if (im->keys[KEY_1] == KEY_PRESS) {
        record_save(&upp->record,
                    get_save_file_name("Record File(*.rec)\0*.rec\0"));
    }
}

static void user_paint_panel_on_show(struct ui *ui) {
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;
    ui_hide((struct ui *) &upp->brushbox);
    ui_hide(&upp->blank);
}

static void user_paint_panel_on_resize(struct ui *ui, int w, int h) {
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;
    ui_resize((struct ui *) &upp->canvas, w,  h);

    ui_resize((struct ui *) &upp->toolbox, w, upp->toolbox.ui.area.h);
    ui_move((struct ui *) &upp->toolbox, 0,
            upp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    ui_resize((struct ui *) &upp->brushbox, w, upp->brushbox.ui.area.h);
    ui_move((struct ui *) &upp->brushbox, 0,
            upp->canvas.ui.area.h - 2 * TOOLBOX_HEIGHT);
}

static void user_paint_panel_on_destroy(struct ui *ui) {
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;

    record_destroy(&upp->record);
}

int user_paint_panel_init(struct user_paint_panel *upp, int w, int h,
                          struct resource_manager *rm) {
    ui_init(&upp->ui, w, h);

    brush_init(&upp->brush_pen, BRUSH_PEN);
    upp->brush_pen_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                                RESOURCE_TEXTURE_ICON_PEN);
    ui_imagebox_init(&upp->brush_pen_icon, 0, 0, upp->brush_pen_image);
    UI_CALLBACK(&upp->brush_pen_icon, press, brush_pen_on_press);

    brush_init(&upp->brush_pencil, BRUSH_PENCIL);
    upp->brush_pencil_image = resource_manager_get(
        rm, RESOURCE_TEXTURE, RESOURCE_TEXTURE_ICON_PENCIL);
    ui_imagebox_init(&upp->brush_pencil_icon, 0, 0, upp->brush_pencil_image);
    UI_CALLBACK(&upp->brush_pencil_icon, press, brush_pencil_on_press);

    brush_init(&upp->brush_eraser, BRUSH_ERASER);
    upp->brush_eraser_image = resource_manager_get(
        rm, RESOURCE_TEXTURE, RESOURCE_TEXTURE_ICON_ERASER);
    ui_imagebox_init(&upp->brush_eraser_icon, 0, 0, upp->brush_eraser_image);
    UI_CALLBACK(&upp->brush_eraser_icon, press, brush_eraser_on_press);

    upp->cur_brush = &upp->brush_pen;

    canvas_init(&upp->canvas, w, h);
    UI_CALLBACK(&upp->canvas, update, canvas_on_update);
    UI_CALLBACK(&upp->canvas, press, canvas_on_press);
    UI_CALLBACK(&upp->canvas, release, canvas_on_release);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->canvas, 0, 0);

    record_init(&upp->record);

    upp->undo_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                           RESOURCE_TEXTURE_ICON_UNDO);
    ui_imagebox_init(&upp->undo, 0, 0, upp->undo_image);
    UI_CALLBACK(&upp->undo, press, undo_on_press);
    UI_CALLBACK(&upp->undo, render, undo_on_render);

    upp->redo_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                           RESOURCE_TEXTURE_ICON_REDO);
    ui_imagebox_init(&upp->redo, 0, 0, upp->redo_image);
    UI_CALLBACK(&upp->redo, press, redo_on_press);
    UI_CALLBACK(&upp->redo, render, redo_on_render);

    ui_imagebox_init(&upp->brush, 0, 0, upp->brush_pen_image);
    UI_CALLBACK(&upp->brush, press, brush_on_press);

    ui_toolbox_init(&upp->toolbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 240);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->undo);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->brush);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->redo);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->toolbox,
                 0, upp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    ui_toolbox_init(&upp->brushbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 240);
    ui_toolbox_add_button(&upp->brushbox, (struct ui *) &upp->brush_pen_icon);
    ui_toolbox_add_button(&upp->brushbox,
                          (struct ui *) &upp->brush_pencil_icon);
    ui_toolbox_add_button(&upp->brushbox,
                          (struct ui *) &upp->brush_eraser_icon);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->brushbox,
                 0, upp->canvas.ui.area.h - 2 * TOOLBOX_HEIGHT);

    ui_init(&upp->blank, w, h - 2 * TOOLBOX_HEIGHT);
    UI_CALLBACK(&upp->blank, press, blank_on_press);
    ui_add_child((struct ui *) upp, &upp->blank, 0, 0);

    UI_CALLBACK(upp, show, user_paint_panel_on_show);
    UI_CALLBACK(upp, resize, user_paint_panel_on_resize);
    UI_CALLBACK(upp, destroy, user_paint_panel_on_destroy);

    return 0;
}
