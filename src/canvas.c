#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sf/utils.h>
#include <sf/array.h>
#include <sf/log.h>

#include "texture.h"
#include "3dmath.h"
#include "canvas.h"
#include "renderer2d.h"


static void canvas_update_content(struct canvas *canvas)
{
    if (!canvas->iscontent_inited) {
        texture_clear(&canvas->content, 0.0f, 0.0f, 0.0f, 0.0f);
        canvas->iscontent_inited = SF_TRUE;
    }

    if (sf_array_cnt(&canvas->plots)) {
        if (!canvas->isbuffet_inited) {
            texture_copy(&canvas->buffer, &canvas->content);
            canvas->isbuffet_inited = SF_TRUE;
        }
        renderer2d_set_render_target(&canvas->content);
        renderer2d_push_viewport(0, 0, canvas->content.w,
                                 canvas->content.h);
        renderer2d_blend_points(canvas->blend_mode, &canvas->buffer,
                                sf_array_head(&canvas->plots),
                                sf_array_cnt(&canvas->plots),
                                canvas->plot_size,
                                canvas->plot_color[0],
                                canvas->plot_color[1],
                                canvas->plot_color[2],
                                canvas->plot_color[3]);
        renderer2d_set_render_target(NULL);
        renderer2d_pop_viewport();

        sf_array_clear(&canvas->plots);
    }
}

static void canvas_render_background(struct canvas *canvas)
{
    /* draw background */
    renderer2d_clear(1.0f, 1.0f, 1.0f, 1.0f);
}

static void canvas_on_render(struct ui *ui)
{
    struct canvas *canvas = (struct canvas *) ui;

    canvas_update_content(canvas);

    canvas_render_background(canvas);

    /* draw content */
    renderer2d_draw_texture(0, 0, 0, 0, &canvas->content, 0, 0, 0, 0);
}

static void canvas_on_resize(struct ui *ui, int w, int h) {
    struct canvas *canvas = (struct canvas *) ui;
    /*float scale = ((float) canvas->viewport.w) / canvas->ui.area.w;*/

    canvas->ui.area.w = w;
    canvas->ui.area.h = h;

    canvas->viewport.w = canvas->ui.area.w;
    canvas->viewport.h = canvas->ui.area.h;

    texture_destroy(&canvas->content);
    texture_destroy(&canvas->buffer);
    canvas->iscontent_inited = SF_FALSE;
    canvas->isbuffet_inited = SF_FALSE;
    texture_init_2d(&canvas->content, w, h);
    texture_init_2d(&canvas->buffer, w, h);
}

static void canvas_on_destroy(struct ui *ui) {
    struct canvas *canvas = (struct canvas *) ui;

    texture_destroy(&canvas->content);
    texture_destroy(&canvas->buffer);
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
    canvas->isploting = SF_FALSE;
    canvas->iscontent_inited = SF_FALSE;
    canvas->isbuffet_inited = SF_FALSE;
    texture_init_2d(&canvas->content, w, h);
    texture_init_2d(&canvas->buffer, w, h);

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
    canvas->isbuffet_inited = SF_FALSE;
    canvas->isploting = SF_FALSE;
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

void canvas_begin_plot(struct canvas *canvas)
{
    assert(canvas->isploting == SF_FALSE);
    canvas->isploting = SF_TRUE;
}

void canvas_plot(struct canvas *canvas, float x, float y)
{
    struct vec2 plot;

    assert(canvas->isploting);

    plot.x = x;
    plot.y = canvas->content.h * 1.0 - y;

    sf_array_push(&canvas->plots, &plot);
}

void canvas_erase(struct canvas *canvas, float x, float y)
{
}

void canvas_end_plot(struct canvas *canvas)
{
    assert(canvas->isploting = SF_TRUE);
    canvas->isploting = SF_FALSE;
    canvas->isbuffet_inited = SF_FALSE;

    canvas_update_content(canvas);
}

void canvas_set_plot_color(struct canvas *canvas, float color[4])
{
    memcpy(canvas->plot_color, color, 4 * sizeof(float));
}

void canvas_set_plot_size(struct canvas *canvas, float size)
{
    canvas->plot_size = size;
}

void canvas_set_blend_mode(struct canvas *canvas, float blend_mode)
{
    canvas->blend_mode = blend_mode;
}

#if 0
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
#endif
