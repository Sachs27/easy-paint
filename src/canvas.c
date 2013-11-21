#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sf/utils.h>
#include <sf/array.h>
#include <sf/log.h>

#include "texture.h"
#include "3dmath.h"
#include "canvas.h"
#include "record.h"


static void canvas_update_content(struct canvas *canvas,
                                  struct renderer2d *r) {
    const uint32_t step = 4;
    uint32_t i;

    if (!canvas->iscontent_inited) {
        renderer2d_set_render_target(r, &canvas->content);
        renderer2d_clear(r, 0.0f, 0.0f, 0.0f, 0.0f);
        canvas->iscontent_inited = SF_TRUE;
        renderer2d_set_render_target(r, NULL);
    }

    for (i = 0; i + step < sf_array_cnt(&canvas->plots); i += step) {
        renderer2d_blend_points(r, &canvas->content,
                                sf_array_nth(&canvas->plots, i), step,
                                canvas->plot_size,
                                canvas->plot_color[0], canvas->plot_color[1],
                                canvas->plot_color[2], canvas->plot_color[3]);
    }

    if (i < sf_array_cnt(&canvas->plots)) {
        renderer2d_blend_points(r, &canvas->content,
                                sf_array_nth(&canvas->plots, i),
                                sf_array_cnt(&canvas->plots) - i,
                                canvas->plot_size,
                                canvas->plot_color[0], canvas->plot_color[1],
                                canvas->plot_color[2], canvas->plot_color[3]);
    }

    sf_array_clear(&canvas->plots);
}

static void canvas_on_render(struct ui *ui, struct renderer2d *r) {
    struct canvas *canvas = (struct canvas *) ui;
    float scale = canvas->viewport.w * 1.0 / canvas->ui.area.w;

    canvas_update_content(canvas, r);

    /* draw background */
    renderer2d_clear(r, 1.0f, 1.0f, 1.0f, 0);

    /* draw content */
    renderer2d_draw_texture(r, 0, 0, 0, 0, &canvas->content, 0, 0, 0, 0);
}

static void canvas_on_resize(struct ui *ui, int w, int h) {
    struct canvas *canvas = (struct canvas *) ui;
    /*float scale = ((float) canvas->viewport.w) / canvas->ui.area.w;*/

    canvas->ui.area.w = w;
    canvas->ui.area.h = h;

    canvas->viewport.w = canvas->ui.area.w;
    canvas->viewport.h = canvas->ui.area.h;

    texture_destroy(&canvas->content);
    canvas->iscontent_inited = SF_FALSE;
    texture_init_2d(&canvas->content, w, h);
}

static void canvas_on_destroy(struct ui *ui) {
    struct canvas *canvas = (struct canvas *) ui;

    texture_destroy(&canvas->content);
    sf_array_destroy(&canvas->plots);
}


int canvas_init(struct canvas *canvas, int w, int h) {
    sf_array_def_t def;

    ui_init((struct ui *) canvas, w, h);

    canvas->background = NULL;
    canvas->viewport.x = 0;
    canvas->viewport.y = 0;
    canvas->viewport.w = w;
    canvas->viewport.h = h;
    canvas->dx = canvas->dy = 0.0f;
    canvas->record = NULL;
    canvas->iscontent_inited = SF_FALSE;
    texture_init_2d(&canvas->content, w, h);

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct vec2);
    sf_array_init(&canvas->plots, &def);

    UI_CALLBACK(canvas, render, canvas_on_render);
    UI_CALLBACK(canvas, resize, canvas_on_resize);
    UI_CALLBACK(canvas, destroy, canvas_on_destroy);

    return 0;
}

void canvas_destroy(struct canvas *canvas) {
    canvas_on_destroy((struct ui *) canvas);
}

void canvas_clear(struct canvas *canvas) {
    canvas->iscontent_inited = SF_FALSE;
}

void canvas_screen_to_canvas(struct canvas *canvas, int x, int y,
                             int *o_x, int *o_y) {
    int xscreen, yscreen;
    ui_get_screen_pos((struct ui *) canvas, &xscreen, &yscreen);
    x -= xscreen;
    y -= yscreen;

    x = ((float) x) / canvas->ui.area.w * canvas->viewport.w;
    y = ((float) y) / canvas->ui.area.h * canvas->viewport.h;
    *o_x = canvas->viewport.x + x;
    *o_y = canvas->viewport.y + y;
}

void canvas_plot(struct canvas *canvas, float x, float y) {
    struct vec2 plot;

    plot.x = x;
    plot.y = canvas->content.h * 1.0 - y;

    sf_array_push(&canvas->plots, &plot);
}

void canvas_set_plot_color(struct canvas *canvas, float color[4]) {
    memcpy(canvas->plot_color, color, 4 * sizeof(float));
}

void canvas_set_plot_size(struct canvas *canvas, float size) {
    canvas->plot_size = size;
}

void canvas_offset(struct canvas *canvas, int xoff, int yoff) {
    float scale = canvas->viewport.w * 1.0 / canvas->ui.area.w;
    int dx, dy;

    canvas->dx += xoff * scale;
    canvas->dy += yoff * scale;

    if (canvas->dx >= 1.0f) {
        dx = floor(canvas->dx);
        canvas->viewport.x += dx;
        canvas->dx -= dx;
    } else if (canvas->dx <= -1.0f) {
        dx = ceil(canvas->dx);
        canvas->viewport.x += dx;
        canvas->dx -= dx;
    }

    if (canvas->dy >= 1.0f) {
        dy = floor(canvas->dy);
        canvas->viewport.y += dy;
        canvas->dy -= dy;
    } else if (canvas->dy <= -1.0f) {
        dy = ceil(canvas->dy);
        canvas->viewport.y += dy;
        canvas->dy -= dy;
    }
}

void canvas_zoom_in(struct canvas *canvas, int cx, int cy) {
    if (canvas->viewport.w < 32 || canvas->viewport.h < 32) {
        return;
    }

    canvas->viewport.w = ((canvas->viewport.x + canvas->viewport.w) - cx)
                       * 0.9f + cx;
    canvas->viewport.h = ((canvas->viewport.y + canvas->viewport.h) - cy)
                       * 0.9f + cy;
    canvas->viewport.x = (canvas->viewport.x - cx) * 0.9f + cx;
    canvas->viewport.y = (canvas->viewport.y - cy) * 0.9f + cy;
    canvas->viewport.w -= canvas->viewport.x;
    canvas->viewport.h -= canvas->viewport.y;
}

void canvas_zoom_out(struct canvas *canvas, int cx, int cy) {
    if (canvas->viewport.w > 2048 || canvas->viewport.h > 2048) {
        return;
    }

    canvas->viewport.w = ((canvas->viewport.x + canvas->viewport.w) - cx)
                       / 0.9f + cx;
    canvas->viewport.h = ((canvas->viewport.y + canvas->viewport.h) - cy)
                       / 0.9f + cy;
    canvas->viewport.x = (canvas->viewport.x - cx) / 0.9f + cx;
    canvas->viewport.y = (canvas->viewport.y - cy) / 0.9f + cy;
    canvas->viewport.w -= canvas->viewport.x;
    canvas->viewport.h -= canvas->viewport.y;
}
