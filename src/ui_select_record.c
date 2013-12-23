#include <stdlib.h>

#include <sf/log.h>
#include <sf/utils.h>

#include "ui_select_record.h"
#include "app.h"
#include "texture.h"
#include "resmgr.h"
#include "record.h"
#include "renderer2d.h"


static void ui_select_record_on_destroy(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    sf_array_destroy(&sr->selections);
    sf_array_destroy(&sr->records);
    sf_array_destroy(&sr->textures);
}

static void generate_textures(struct ui_select_record *sr)
{
    struct texture tex;
    int i;
    int isselected = 0;

    sf_array_clear(&sr->textures);
    sf_array_clear(&sr->selections);
    for (i = 0; i < sf_array_cnt(&sr->records); ++i) {
        record_to_texture(*(struct record **) sf_array_nth(&sr->records, i),
                          &tex, sr->texture_w, sr->texture_h);
        sf_array_push(&sr->textures, &tex);
        sf_array_push(&sr->selections, &isselected);
    }
}

static void ui_select_record_on_show(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;
    int i, nrecords;
    struct record **records;

    nrecords = rm_get_user_define_record_count();
    records = sf_alloc(sizeof(struct record *) * nrecords);
    rm_get_all_user_define_records(records, nrecords);

    sf_array_clear(&sr->records);
    for (i = 0; i < nrecords; ++i) {
        sf_array_push(&sr->records, records + i);
    }
    sf_free(records);

    generate_textures(sr);
}

static void ui_select_record_on_hide(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    sf_array_clear(&sr->records);
    sf_array_clear(&sr->textures);
}

static void ui_select_record_on_render(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;
    int x, y, i;

    renderer2d_clear(0.7, 0.7, 0.7, 0.0);

    x = sr->padding;
    y = sr->padding + sr->yoffset;
    for (i = 0; i < sf_array_cnt(&sr->textures); ++i) {
        struct sf_rect r = {x, y, sr->texture_w, sr->texture_h};
        if (sf_rect_isintersect(&r, &ui->area)) {
            renderer2d_fill_rect(x, y, sr->texture_w, sr->texture_h,
                                 255, 255, 255, 255);
            renderer2d_draw_texture(x, y, sr->texture_w, sr->texture_h,
                                    sf_array_nth(&sr->textures, i),
                                    0, 0, 0, 0);

            if (sr->isselecting) {
                int tx = x + sr->texture_w - TOOLBOX_HEIGHT;
                int ty = y;
                if (*(int *) sf_array_nth(&sr->selections, i)) {
                    renderer2d_draw_texture(tx, ty, TOOLBOX_HEIGHT,
                                            TOOLBOX_HEIGHT,
                                            sr->checked_image,
                                            0, 0, 0, 0);
                } else {
                    renderer2d_draw_texture(tx, ty, TOOLBOX_HEIGHT,
                                            TOOLBOX_HEIGHT,
                                            sr->unchecked_image,
                                            0, 0, 0, 0);
                }
            }
        }
        x += sr->texture_w + sr->padding;
        if (x + sr->texture_w > ui->area.w) {
            x = sr->padding;
            y += sr->texture_h + sr->padding;
        }
    }
}

static void ui_select_record_on_resize(struct ui *ui, int w, int h)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    sr->isselecting = 0;
    sr->yoffset = 0;

    sr->texture_w = (w - 3 * sr->padding) / 3;
    if (sr->texture_w < 64) {
        sr->texture_w = 64;
    } else if (sr->texture_w - 64 < 128) {
        sr->texture_w = 128;
    } else {
       sr->texture_w = 256;
    }
    sr->texture_h = sr->texture_w;

    generate_textures(sr);

    ui_move((struct ui *) &sr->ib_new, 0, sr->yoffset);

    ui_resize((struct ui *) &sr->tb, w, TOOLBOX_HEIGHT);
    ui_move((struct ui *) &sr->tb, 0, h - sr->tb.ui.area.h);
}

static int ui_select_record_on_tap(struct ui *ui, int x, int y)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;
    int i, tx, ty;

    tx = sr->padding;
    ty = sr->padding + sr->yoffset;
    for (i = 0; i < sf_array_cnt(&sr->textures); ++i) {
        struct sf_rect area;
        area.x = tx;
        area.y = ty;
        area.w = sr->texture_w;
        area.h = sr->texture_h;
        if (sf_rect_iscontain(&area, x, y)) {
            if (sr->isselecting) {
            /* in selection mode, add to/remove from selections */
                int *ptr = sf_array_nth(&sr->selections, i);

                *ptr = !*ptr;
            } else {
                user_paint_panel_set_record(&g_app.upp,
                                            *(struct record **)
                                             sf_array_nth(&sr->records, i));
                app_change_stage(APP_STAGE_DOODLE);
            }

            break;
        }
        tx += sr->texture_w + sr->padding;
        if (tx + sr->texture_w > ui->area.w) {
            tx = sr->padding;
            ty += sr->texture_h + sr->padding;
        }
    }

    return 0;
}

static int ui_select_record_on_press(struct ui *ui, int x, int y)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    sr->ispressed = 1;
    sr->lastx = x;
    sr->lasty = y;
    return 0;
}

static int ui_select_record_on_long_press(struct ui *ui, int x, int y)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;
    int i, tx, ty;

    sr->ispressed = 0;
    sr->isselecting = 1;

    tx = sr->padding;
    ty = sr->padding + sr->yoffset;
    for (i = 0; i < sf_array_cnt(&sr->textures); ++i) {
        struct sf_rect area;
        area.x = tx;
        area.y = ty;
        area.w = sr->texture_w;
        area.h = sr->texture_h;
        if (sf_rect_iscontain(&area, x, y)) {
            int *ptr = sf_array_nth(&sr->selections, i);

            *ptr = !*ptr;
            break;
        }
        tx += sr->texture_w + sr->padding;
        if (tx + sr->texture_w > ui->area.w) {
            tx = sr->padding;
            ty += sr->texture_h + sr->padding;
        }
    }

    return 0;
}

static void ui_select_record_release(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    sr->ispressed = 0;
}

static void ui_select_record_on_update(struct ui *ui, struct input_manager *im,
                                       double dt)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;

    if (sr->ispressed) {
        int dx, dy;
        int x, y;

        ui_get_screen_pos(ui, &x, &y);

        x = im->mouse.x - x;
        y = im->mouse.y - y;
        dx = x - sr->lastx;
        dy = y - sr->lasty;
        sr->lastx = x;
        sr->lasty = y;

        if (dx != 0 || dy != 0) {
            if (abs(dy) > abs(dx)) {
                sr->yoffset += dy;

                if (sr->yoffset < 0) {
                    int i;
                    int offset = 0;

                    x = sr->padding;
                    y = sr->padding;
                    for (i = 0; i < sf_array_cnt(&sr->textures); ++i) {
                        if (y > offset) {
                            offset = y;
                        }
                        x += sr->texture_w + sr->padding;
                        if (x + sr->texture_w > ui->area.w) {
                            x = sr->padding;
                            y += sr->texture_h + sr->padding;
                        }
                    }
                    offset += sr->texture_h + sr->padding;
                    y = sr->ui.area.h - offset;
                    if (sr->yoffset < y) {
                        sr->yoffset = y;
                    }
                }

                if (sr->yoffset >= 0) {
                    sr->yoffset = 0;
                }

                ui_move((struct ui *) &sr->ib_new, 0, sr->yoffset);
            }
        }
    }

    if (sr->isselecting) {
        ui_show((struct ui *) &sr->tb);
    } else {
        ui_hide((struct ui *) &sr->tb);
    }
}

static void texture_free(void *elt)
{
    texture_destroy(elt);
}


int ui_select_record_init(struct ui_select_record *sr, int w, int h)
{
    sf_array_def_t def;

    ui_init((struct ui *) sr, w, h);

    sr->ispressed = 0;
    sr->isselecting = 0;
    sr->yoffset = 0;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(int);
    sf_array_init(&sr->selections, &def);

    def.size = sizeof(struct record *);
    sf_array_init(&sr->records, &def);

    def.size = sizeof(struct texture);
    def.free = texture_free;
    sf_array_init(&sr->textures, &def);

    sr->padding = 10;
    sr->texture_w = (w - 3 * sr->padding) / 3;
    sr->texture_h = sr->texture_w;

    sr->new_image = rm_load_texture(RES_TEXTURE_ICON_NEW);
    ui_imagebox_init(&sr->ib_new, 48, 48, sr->new_image);
    ui_add_child((struct ui *) sr, (struct ui *) &sr->ib_new,
                 0, sr->yoffset);

    sr->checked_image = rm_load_texture(RES_TEXTURE_ICON_CHECKED);
    sr->unchecked_image = rm_load_texture(RES_TEXTURE_ICON_UNCHECKED);

    ui_toolbox_init(&sr->tb, w, TOOLBOX_HEIGHT, 128, 128, 128, 255);
    ui_add_child((struct ui *) sr, (struct ui *) &sr->tb,
                 0, h - sr->tb.ui.area.h);

    UI_CALLBACK(sr, destroy, ui_select_record_on_destroy);
    UI_CALLBACK(sr, show, ui_select_record_on_show);
    UI_CALLBACK(sr, hide, ui_select_record_on_hide);
    UI_CALLBACK(sr, render, ui_select_record_on_render);
    UI_CALLBACK(sr, update, ui_select_record_on_update);
    UI_CALLBACK(sr, resize, ui_select_record_on_resize);
    UI_CALLBACK(sr, press, ui_select_record_on_press);
    UI_CALLBACK(sr, long_press, ui_select_record_on_long_press);
    UI_CALLBACK(sr, tap, ui_select_record_on_tap);
    UI_CALLBACK(sr, release, ui_select_record_release);

    return SF_OK;
}
