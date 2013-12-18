#include <sf/utils.h>

#include "ui_select_record.h"
#include "texture.h"
#include "resmgr.h"
#include "record.h"
#include "renderer2d.h"


static void generate_textures(struct ui_select_record *sr)
{
    struct texture tex;
    int i;

    sf_array_clear(&sr->textures);
    for (i = 0; i < sf_array_cnt(&sr->records); ++i) {
        record_to_texture(*(struct record **) sf_array_nth(&sr->records, i),
                          &tex, sr->texture_w, sr->texture_h);
        sf_array_push(&sr->textures, &tex);
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

static void ui_select_record_on_render(struct ui *ui)
{
    struct ui_select_record *sr = (struct ui_select_record *) ui;
    int x, y, i;

    renderer2d_clear(0.7, 0.7, 0.7, 0.0);

    x = sr->padding;
    y = sr->padding;
    for (i = 0; i < sf_array_cnt(&sr->textures); ++i) {
        renderer2d_fill_rect(x, y, sr->texture_w, sr->texture_h,
                             255, 255, 255, 255);
        renderer2d_draw_texture(x, y, sr->texture_w, sr->texture_h,
                                sf_array_nth(&sr->textures, i), 0, 0, 0, 0);
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
}

static void texture_free(void *elt)
{
    texture_destroy(elt);
}


int ui_select_record_init(struct ui_select_record *sr, int w, int h)
{
    sf_array_def_t def;

    ui_init((struct ui *) sr, w, h);

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct record *);
    sf_array_init(&sr->records, &def);

    def.size = sizeof(struct texture);
    def.free = texture_free;
    sf_array_init(&sr->textures, &def);

    sr->padding = 10;
    sr->texture_w = (w - 3 * sr->padding) / 3;
    sr->texture_h = sr->texture_w;

    UI_CALLBACK(sr, show, ui_select_record_on_show);
    UI_CALLBACK(sr, render, ui_select_record_on_render);
    UI_CALLBACK(sr, resize, ui_select_record_on_resize);

    return SF_OK;
}
