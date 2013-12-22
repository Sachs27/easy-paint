#include <assert.h>
#include <stddef.h>
#include <time.h>

#include <sf/utils.h>

#include "app.h"
#include "user_paint_panel.h"
#include "resmgr.h"
#include "renderer2d.h"


/*
 * The following callback function use global variable g_app.
 */
static void undo_on_render(struct ui *ui)
{
    struct ui_imagebox *undo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(undo, struct user_paint_panel, undo);

    if (undo->image == NULL) {
        return;
    }

    if (record_canundo(upp->record)) {
        renderer2d_draw_texture(0, 0, 0, 0, undo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(0, 0, 0, 0,
                                           undo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static int undo_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *undo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(undo, struct user_paint_panel, undo);

    if (upp->blank.state == UI_STATE_SHOW) {
        ui_hide(&upp->blank);
    }

    if (record_canundo(upp->record)) {
        upp->cansave = 1;
        record_undo(upp->record, &upp->canvas);
    }
    return 0;
}

static void redo_on_render(struct ui *ui)
{
    struct ui_imagebox *redo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(redo, struct user_paint_panel, redo);

    if (redo->image == NULL) {
        return;
    }

    if (record_canredo(upp->record)) {
        renderer2d_draw_texture(0, 0, 0, 0, redo->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(0, 0, 0, 0,
                                           redo->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static int redo_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *redo = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(redo, struct user_paint_panel, redo);

    if (upp->blank.state == UI_STATE_SHOW) {
        ui_hide(&upp->blank);
    }

    if (record_canredo(upp->record)) {
        upp->cansave = 1;
        record_redo(upp->record, &upp->canvas);
    }

    return 0;
}

static int brush_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *brush = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(brush, struct user_paint_panel, brush);

    if (upp->blank.state == UI_STATE_SHOW) {
        ui_hide(&upp->blank);
    } else {
        ui_show(&upp->blank);
    }
    return 0;
}

static int brush_pen_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *brush_pen_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_pen_icon,
                                                   struct user_paint_panel,
                                                   brush_pen_icon);
    upp->cur_brush = &upp->brush_pen;
    ui_imagebox_set_image(&upp->brush, upp->brush_pen_icon.image);
    ui_hide(&upp->blank);
    return 0;
}

static int brush_pencil_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *brush_pencil_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_pencil_icon,
                                                   struct user_paint_panel,
                                                   brush_pencil_icon);
    upp->cur_brush = &upp->brush_pencil;
    ui_imagebox_set_image(&upp->brush, upp->brush_pencil_icon.image);
    ui_hide(&upp->blank);
    return 0;
}

static int brush_eraser_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *brush_eraser_icon = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp = sf_container_of(brush_eraser_icon,
                                                   struct user_paint_panel,
                                                   brush_eraser_icon);
    upp->cur_brush = &upp->brush_eraser;
    ui_imagebox_set_image(&upp->brush, upp->brush_eraser_icon.image);
    ui_hide(&upp->blank);
    return 0;
}

static int blank_on_press(struct ui *blank, int x, int y)
{
    struct user_paint_panel *upp =
        sf_container_of(blank, struct user_paint_panel, blank);

    ui_hide(&upp->blank);
    return 0;
}

static int canvas_lastx = -1;
static int canvas_lasty = -1;

static int canvas_on_press(struct ui *ui, int x, int y)
{
    struct canvas *canvas = (struct canvas *) ui;
    struct user_paint_panel *upp =
        sf_container_of(canvas, struct user_paint_panel, canvas);
    int xscreen, yscreen;

    ui_color_picker_set_color(&upp->color_picker, upp->cur_brush->color);

    ui_get_screen_pos(ui, &xscreen, &yscreen);
    canvas_screen_to_canvas(canvas, x + xscreen, y + yscreen,
                            &canvas_lastx, &canvas_lasty);

    canvas_begin_plot(canvas);

    if (upp->record) {
        record_begin_plot(upp->record, upp->cur_brush);
    }

    return 0;
}

static void canvas_on_release(struct ui *ui)
{
    struct canvas *canvas = (struct canvas *) ui;
    struct user_paint_panel *upp =
        sf_container_of(canvas, struct user_paint_panel, canvas);

    canvas_end_plot(canvas);

    if (upp->record) {
        record_end_plot(upp->record);
    }
}

static void canvas_on_update(struct ui *ui, struct input_manager *im,
                             double dt)
{
    struct canvas *canvas = (struct canvas *) ui;
    struct user_paint_panel *upp =
        sf_container_of(canvas, struct user_paint_panel, canvas);

    if (canvas->isploting) {
        sf_list_t *buf = &im->mb_left_buffer;
        sf_list_iter_t iter;
        int mx, my;

        if (sf_list_begin(buf, &iter)) do {
            struct ivec2 *pos = sf_list_iter_elt(&iter);
            canvas_screen_to_canvas(canvas, pos->x, pos->y, &mx, &my);

            if (mx != canvas_lastx || my != canvas_lasty) {
                upp->cansave = 1;
                brush_drawline(upp->cur_brush, canvas,
                               canvas_lastx, canvas_lasty, mx, my);
                if (upp->record) {
                    record_drawline(upp->record, canvas_lastx, canvas_lasty,
                                    mx, my);
                }
                canvas_lastx = mx;
                canvas_lasty = my;
            }
        } while (sf_list_iter_next(&iter));
    }
#if 0
    if (im->keys[KEY_1] == KEY_PRESS) {
        record_save(&upp->record,
                    get_save_file_name("Record File(*.rec)\0*.rec\0"));
    }
#endif
}

static int replay_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *replay = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(replay, struct user_paint_panel, replay);

    if (!record_canundo(upp->record)) {
        return 0;
    }

    upp->isplaying = 1;
    ui_hide((struct ui *) &upp->canvas);
    ui_hide((struct ui *) &upp->toolbox);
    ui_hide((struct ui *) &upp->blank);

    ui_show((struct ui *) &upp->urp);
    ui_replay_panel_set_record(&upp->urp, upp->record);
    ui_replay_panel_play(&upp->urp);

    return 0;
}

static void save_on_render(struct ui *ui)
{
    struct ui_imagebox *save = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(save, struct user_paint_panel, save);

    if (upp->cansave) {
        renderer2d_draw_texture(0, 0, 0, 0, save->image, 0, 0, 0, 0);
    } else {
        renderer2d_draw_texture_with_color(0, 0, 0, 0,
                                           save->image, 0, 0, 0, 0,
                                           128, 128, 128);
    }
}

static int save_on_press(struct ui *ui, int x, int y)
{
    struct ui_imagebox *save = (struct ui_imagebox *) ui;
    struct user_paint_panel *upp =
        sf_container_of(save, struct user_paint_panel, save);

    if (!upp->cansave || !upp->record) {
        return 0;
    }

    upp->cansave = 0;

    if (upp->record) {
        time_t rawtime = time(0);
        char buf[64];

        strftime(buf, 64, "%F-%H-%M-%S.epr", localtime(&rawtime));

        rm_save_as_user_define_record(upp->record, buf);
    }

    return 0;
}

static void user_paint_panel_on_show(struct ui *ui)
{
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;
    ui_hide(&upp->blank);
    ui_hide((struct ui *) &upp->urp);

    canvas_clear(&upp->canvas);

    /*upp->record = rm_load_last_record();*/

    upp->isadject = 1;
}

static void user_paint_panel_on_hide(struct ui *ui)
{
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;

    if (upp->cansave) {
        if (upp->record == rm_load_last_record()) {
            rm_save_user_define_record(upp->record);
        } else if (upp->record) {
            time_t rawtime = time(0);
            char buf[64];

            strftime(buf, 64, "%F-%H-%M-%S.epr", localtime(&rawtime));

            rm_save_as_user_define_record(upp->record, buf);
        }
        upp->cansave = 0;
    }

    upp->record = NULL;
}

static void user_paint_panel_on_resize(struct ui *ui, int w, int h)
{
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;

    ui_resize((struct ui *) &upp->canvas, w,  h - TOOLBOX_HEIGHT);

    upp->isadject = 1;
    upp->isresizing = 1;

    ui_resize((struct ui *) &upp->urp, w,  h);

    ui_resize((struct ui *) &upp->undo, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->redo, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->brush, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->brushbox, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->brush_pen_icon,
              TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->brush_pencil_icon,
              TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->brush_eraser_icon,
              TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->save, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);
    ui_resize((struct ui *) &upp->replay, TOOLBOX_HEIGHT, TOOLBOX_HEIGHT);

    ui_resize((struct ui *) &upp->toolbox, w, TOOLBOX_HEIGHT);
    ui_move((struct ui *) &upp->toolbox, 0, upp->canvas.ui.area.h);

    ui_resize(&upp->blank, w, h - upp->toolbox.ui.area.h);

    ui_resize((struct ui *) &upp->brushbox, w, TOOLBOX_HEIGHT);
    ui_move((struct ui *) &upp->brushbox, 0,
            upp->toolbox.ui.area.y - upp->brushbox.ui.area.h);

    ui_resize((struct ui *) &upp->color_picker,
              w, upp->color_picker.ui.area.h);
    ui_move((struct ui *) &upp->color_picker, 0,
            upp->brushbox.ui.area.y - upp->color_picker.ui.area.h);
}

static void user_paint_panel_on_update(struct ui *ui, struct input_manager *im,
                                       double dt)
{
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;

    if (upp->isplaying) {
        if (upp->urp.isstop) {
            upp->isplaying = 0;
            ui_hide((struct ui *) &upp->urp);
            ui_show((struct ui *) &upp->canvas);
            ui_show((struct ui *) &upp->toolbox);
        }
    } else {
        ui_color_picker_get_color(&upp->color_picker, upp->cur_brush->color);
    }

}

static void user_paint_panel_on_render(struct ui *ui)
{
    struct user_paint_panel *upp = (struct user_paint_panel *) ui;

    if (upp->record) {
        int needredraw = 0;
        if (upp->isadject) {
            record_adjust(upp->record, upp->canvas.ui.area.w, upp->canvas.ui.area.h);
            upp->isadject = 0;
            needredraw = 1;
        }
        if (upp->isresizing) {
            upp->isresizing = 0;
            needredraw = 1;
        }
        if (needredraw && record_canundo(upp->record)) {
            record_undo(upp->record, &upp->canvas);
            record_redo(upp->record, &upp->canvas);
        }
    }
}

static void user_paint_panel_on_destroy(struct ui *ui)
{
    user_paint_panel_on_hide(ui);
}

int user_paint_panel_init(struct user_paint_panel *upp, int w, int h)
{
    ui_init(&upp->ui, w, h);

    brush_init(&upp->brush_pen, BRUSH_PEN);
    upp->brush_pen_image = rm_load_texture(RES_TEXTURE_ICON_PEN);
    ui_imagebox_init(&upp->brush_pen_icon, 0, 0, upp->brush_pen_image);
    UI_CALLBACK(&upp->brush_pen_icon, press, brush_pen_on_press);

    brush_init(&upp->brush_pencil, BRUSH_PENCIL);
    upp->brush_pencil_image = rm_load_texture(RES_TEXTURE_ICON_PENCIL);
    ui_imagebox_init(&upp->brush_pencil_icon, 0, 0, upp->brush_pencil_image);
    UI_CALLBACK(&upp->brush_pencil_icon, press, brush_pencil_on_press);

    brush_init(&upp->brush_eraser, BRUSH_ERASER);
    upp->brush_eraser_image = rm_load_texture(RES_TEXTURE_ICON_ERASER);
    ui_imagebox_init(&upp->brush_eraser_icon, 0, 0, upp->brush_eraser_image);
    UI_CALLBACK(&upp->brush_eraser_icon, press, brush_eraser_on_press);

    upp->cur_brush = &upp->brush_pen;

    canvas_init(&upp->canvas, w, h - TOOLBOX_HEIGHT);
    UI_CALLBACK(&upp->canvas, update, canvas_on_update);
    UI_CALLBACK(&upp->canvas, press, canvas_on_press);
    UI_CALLBACK(&upp->canvas, release, canvas_on_release);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->canvas, 0, 0);

    upp->record = NULL;
    upp->isadject = 0;
    upp->isplaying = 0;
    upp->isresizing = 0;
    upp->cansave = 0;

    ui_replay_panel_init(&upp->urp, w, h);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->urp, 0, 0);

    upp->undo_image = rm_load_texture(RES_TEXTURE_ICON_UNDO);
    ui_imagebox_init(&upp->undo, 0, 0, upp->undo_image);
    UI_CALLBACK(&upp->undo, press, undo_on_press);
    UI_CALLBACK(&upp->undo, render, undo_on_render);

    upp->redo_image = rm_load_texture(RES_TEXTURE_ICON_REDO);
    ui_imagebox_init(&upp->redo, 0, 0, upp->redo_image);
    UI_CALLBACK(&upp->redo, press, redo_on_press);
    UI_CALLBACK(&upp->redo, render, redo_on_render);

    ui_imagebox_init(&upp->brush, 0, 0, upp->brush_pen_image);
    UI_CALLBACK(&upp->brush, press, brush_on_press);

    upp->replay_image = rm_load_texture(RES_TEXTURE_ICON_PLAY);
    ui_imagebox_init(&upp->replay, 0, 0, upp->replay_image);
    UI_CALLBACK(&upp->replay, press, replay_on_press);

    upp->save_image = rm_load_texture(RES_TEXTURE_ICON_SAVE);
    ui_imagebox_init(&upp->save, 48, 48, upp->save_image);
    UI_CALLBACK(&upp->save, press, save_on_press);
    UI_CALLBACK(&upp->save, render, save_on_render);

    ui_toolbox_init(&upp->toolbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 255);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->brush);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->undo);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->redo);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->save);
    ui_toolbox_add_button(&upp->toolbox, (struct ui *) &upp->replay);
    ui_add_child((struct ui *) upp, (struct ui *) &upp->toolbox,
                 0, upp->canvas.ui.area.h);

    ui_init(&upp->blank, w, h - upp->toolbox.ui.area.h);
    UI_CALLBACK(&upp->blank, press, blank_on_press);
    ui_add_child((struct ui *) upp, &upp->blank, 0, 0);

    ui_toolbox_init(&upp->brushbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 240);
    ui_toolbox_add_button(&upp->brushbox, (struct ui *) &upp->brush_pen_icon);
    ui_toolbox_add_button(&upp->brushbox,
                          (struct ui *) &upp->brush_pencil_icon);
    ui_toolbox_add_button(&upp->brushbox,
                          (struct ui *) &upp->brush_eraser_icon);
    ui_add_child(&upp->blank, (struct ui *) &upp->brushbox,
                 0, upp->toolbox.ui.area.y - TOOLBOX_HEIGHT);


    ui_color_picker_init(&upp->color_picker, w, 281);
    ui_color_picker_set_color(&upp->color_picker, upp->cur_brush->color);
    ui_add_child(&upp->blank, (struct ui *) &upp->color_picker,
                 0, upp->brushbox.ui.area.y - upp->color_picker.ui.area.h);

    UI_CALLBACK(upp, show, user_paint_panel_on_show);
    UI_CALLBACK(upp, hide, user_paint_panel_on_hide);
    UI_CALLBACK(upp, resize, user_paint_panel_on_resize);
    UI_CALLBACK(upp, update, user_paint_panel_on_update);
    UI_CALLBACK(upp, render, user_paint_panel_on_render);
    UI_CALLBACK(upp, destroy, user_paint_panel_on_destroy);

    return 0;
}

void user_paint_panel_set_record(struct user_paint_panel *upp,
                                 struct record *r)
{
    upp->record = r;
}
