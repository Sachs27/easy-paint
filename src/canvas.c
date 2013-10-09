#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <sf_utils.h>
#include <sf_array.h>
#include <sf_debug.h>

#include "canvas.h"


#define CANVAS_RECORD_NALLOC 2048

struct record {
    struct ivec2    position;
    uint8_t         new_color[4];
    uint8_t         old_color[4];
};


#define CANVAS_TILE_WIDTH 512

#define CANVAS_TILE_HEIGHT 512

struct canvas_tile {
    struct sf_rect      area;

    int                 isdirty;
    struct sf_rect      dirty_rect;

    uint8_t            *colors;     /* R-G-B-A (8-bit each) */

    struct texture     *texture;
};


static void canvas_tile_init(struct canvas_tile *ct, int x, int y) {
    ct->texture = texture_create_2d(CANVAS_TILE_WIDTH, CANVAS_TILE_HEIGHT);
    /* clear the texture's content */
    /*renderer2d_set_render_target(g_app.renderer2d, ct->texture);*/
    /*renderer2d_clear(g_app.renderer2d, 0, 0, 0, 0);*/
    /*renderer2d_set_render_target(g_app.renderer2d, NULL);*/

    ct->area.x = x;
    ct->area.y = y;
    ct->area.w = ct->texture->w;
    ct->area.h = ct->texture->h;
    ct->isdirty = 0;
    ct->colors = calloc(ct->texture->w * ct->texture->h, 4 * sizeof(uint8_t));
}

static void canvas_tile_plot(struct canvas_tile *ct, int x, int y,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint8_t *color;

    if (ct->isdirty == 0) {
        ct->isdirty = 1;
        ct->dirty_rect.x = 0;
        ct->dirty_rect.y = 0;
        ct->dirty_rect.w = 0;
        ct->dirty_rect.h = 0;
    }
    /* conver coordinate to canvas tile's */
    x -= ct->area.x;
    y -= ct->area.y;

    color = ct->colors + 4 * (y * ct->texture->h + x);
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;

    if (!sf_rect_iscontain(&ct->dirty_rect, x, y)) {
        /* calc the dirty rect */
        if (ct->dirty_rect.w == 0) {
            ct->dirty_rect.x = x;
            ct->dirty_rect.y = y;
            ct->dirty_rect.w = 1;
            ct->dirty_rect.h = 1;
        } else {
            int x1 = ct->dirty_rect.x + ct->dirty_rect.w;
            int y1 = ct->dirty_rect.y + ct->dirty_rect.h;

            if (x < ct->dirty_rect.x) {
                ct->dirty_rect.w += ct->dirty_rect.x - x;
                ct->dirty_rect.x = x;
            } else if (x >= x1) {
                ct->dirty_rect.w += x - x1 + 1;
            }

            if (y < ct->dirty_rect.y) {
                ct->dirty_rect.h += ct->dirty_rect.y - y;
                ct->dirty_rect.y = y;
            } else if (y >= y1) {
                ct->dirty_rect.h += y - y1 + 1;
            }
        }
    }
}

static void canvas_update_tile(struct canvas *canvas, struct canvas_tile *ct) {
    uint32_t colors[ct->dirty_rect.w * ct->dirty_rect.h];
    uint32_t *row;
    uint32_t *src_row;
    int i;

    src_row = ((uint32_t *) (ct->colors))
                                + ct->dirty_rect.y * ct->texture->w
                                + ct->dirty_rect.x;
    row = colors;

    for (i = 0; i < ct->dirty_rect.h; ++i) {
        memcpy(row, src_row, ct->dirty_rect.w * sizeof(uint32_t));
        row += ct->dirty_rect.w;
        src_row += ct->texture->w;
    }
    /* use dirty rect */
    glBindTexture(GL_TEXTURE_2D, ct->texture->tid);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    ct->dirty_rect.x, ct->dirty_rect.y,
                    ct->dirty_rect.w, ct->dirty_rect.h,
                    GL_RGBA, GL_UNSIGNED_BYTE, colors);
    glBindTexture(GL_TEXTURE_2D, 0);
    ct->isdirty = 0;
}

/**
 * (x, y) is the point which the new tile must contain.
 */
static struct canvas_tile *canvas_add_tile(struct canvas *canvas,
                                           int x, int y) {
    int xtile = 0, ytile = 0, xstep, ystep;
    struct canvas_tile ct;

    xstep = x > 0 ? CANVAS_TILE_WIDTH : -CANVAS_TILE_WIDTH;
    ystep = y > 0 ? CANVAS_TILE_HEIGHT : -CANVAS_TILE_HEIGHT;

    while (!((x - xtile) >= 0 && (x - xtile) < CANVAS_TILE_WIDTH)) {
        xtile += xstep;
    }

    while (!((y - ytile) >= 0 && (y - ytile) < CANVAS_TILE_WIDTH)) {
        ytile += ystep;
    }

    canvas_tile_init(&ct, xtile, ytile);

    dprintf("canvas has %d tiles\n", canvas->tiles->nelts + 1);

    return sf_list_push(canvas->tiles, &ct);
}

static void canvas_record(struct canvas *canvas, int x, int y,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    struct record record;
    uint8_t old_color[4];

    if (canvas->isrecording == 0) {
        return;
    }

    canvas_pick(canvas, x, y, old_color, old_color + 1, old_color + 2,
                old_color + 3);

    record.position.x = x;
    record.position.y = y;
    record.old_color[0] = old_color[0];
    record.old_color[1] = old_color[1];
    record.old_color[2] = old_color[2];
    record.old_color[3] = old_color[3];
    record.new_color[0] = r;
    record.new_color[1] = g;
    record.new_color[2] = b;
    record.new_color[3] = a;

    sf_array_push(*(struct sf_array **)
                   SF_ARRAY_NTH(canvas->segments, canvas->cur_segment),
                  &record);
}

static void canvas_on_update(struct canvas *canvas, struct input_manager *im,
                             double dt) {
    int mx, my;

    canvas_screen_to_canvas(canvas, im->mouse.x, im->mouse.y, &mx, &my);

    if (canvas->cur_brush && canvas->isrecording) {
        if (sf_rect_iscontain(&canvas->viewport, mx, my)
            && (mx != canvas->lastx || my != canvas->lasty)) {
            brush_drawline(canvas->cur_brush, canvas,
                           canvas->lastx, canvas->lasty, mx, my);
        }

        canvas->lastx = mx;
        canvas->lasty = my;
    }
}

static void canvas_on_render(struct canvas *canvas,
                             struct renderer2d *renderer2d) {
    float scale = canvas->viewport.w * 1.0 / canvas->ui.area.w;
    /* draw background */
    renderer2d_clear(renderer2d, 255, 255, 255, 0);
    /* draw pixels */
    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        int x0, y0, x1, y1, x2, y2, x3, y3;

        if (!sf_rect_isintersect(&canvas->viewport, &ct->area)) {
            continue;
        }

        if (ct->isdirty) {
            canvas_update_tile(canvas, ct);
        }
#define CLAMP(a, min, max) do {         \
    if ((a) < (min)) {                  \
        (a) = (min);                    \
    } else if ((a) > (max)) {           \
        (a) = (max);                    \
    }                                   \
}while (0)
        x0 = ct->area.x - canvas->viewport.x;
        y0 = ct->area.y - canvas->viewport.y;
        x1 = x0 + ct->texture->w;
        y1 = y0 + ct->texture->h;
        CLAMP(x0, 0, canvas->viewport.w);
        CLAMP(y0, 0, canvas->viewport.h);
        CLAMP(x1, 0, canvas->viewport.w);
        CLAMP(y1, 0, canvas->viewport.h);
        x0 /= scale;
        y0 /= scale;
        x1 /= scale;
        y1 /= scale;

        x2 = canvas->viewport.x - ct->area.x;
        y2 = canvas->viewport.y - ct->area.y;
        x3 = x2 + canvas->viewport.w;
        y3 = y2 + canvas->viewport.h;
        CLAMP(x2, 0, ct->texture->w);
        CLAMP(y2, 0, ct->texture->h);
        CLAMP(x3, 0, ct->texture->w);
        CLAMP(y3, 0, ct->texture->h);
#undef CLAMP
        renderer2d_draw_texture(renderer2d,
                                x0, y0, x1 - x0, y1 - y0,
                                ct->texture,
                                x2, y2, x3 - x2, y3 - y2);
    SF_LIST_END();
}

static void canvas_on_press(struct canvas *canvas,
                            int n, int x[n], int y[n]) {
    if (!canvas->isrecording && n == 1) {
        canvas_screen_to_canvas(canvas, x[0] + canvas->ui.area.x,
                                y[0] + canvas->ui.area.y,
                                &canvas->lastx, &canvas->lasty);
        canvas_record_begin(canvas);
    }
}

static void canvas_on_release(struct canvas *canvas) {
    if (canvas->isrecording) {
        canvas_record_end(canvas);
    }
}


struct canvas *canvas_create(int w, int h) {
    struct canvas *canvas;

    canvas = malloc(sizeof(*canvas));
    assert(canvas != NULL);
    ui_init((struct ui *) canvas, w, h);
    canvas->background = NULL;
    canvas->viewport.x = 0;
    canvas->viewport.y = 0;
    canvas->viewport.w = w;
    canvas->viewport.h = h;
    canvas->lastx = canvas->lasty = 0;
    canvas->dx = canvas->dy = 0.0f;
    canvas->tiles = sf_list_create(sizeof(struct canvas_tile));
    canvas->isrecording = 0;
    canvas->cur_segment = -1;
    canvas->segments = sf_array_create(sizeof(struct sf_array *),
                                       SF_ARRAY_NALLOC);

    UI_CALLBACK(canvas, update, canvas_on_update);
    UI_CALLBACK(canvas, render, canvas_on_render);
    UI_CALLBACK(canvas, press, canvas_on_press);
    UI_CALLBACK(canvas, release, canvas_on_release);

    return canvas;
}

void canvas_resize(struct canvas *canvas, int w, int h) {
    float scale = ((float) canvas->viewport.w) / canvas->ui.area.w;

    canvas->ui.area.w = w;
    canvas->ui.area.h = h;

    canvas->viewport.w = scale * canvas->ui.area.w;
    canvas->viewport.h = scale * canvas->ui.area.h;
}

void canvas_screen_to_canvas(struct canvas *canvas, int x, int y,
                             int *o_x, int *o_y) {
    x -= canvas->ui.area.x;
    y -= canvas->ui.area.y;

    x = ((float) x) / canvas->ui.area.w * canvas->viewport.w;
    y = ((float) y) / canvas->ui.area.h * canvas->viewport.h;
    *o_x = canvas->viewport.x + x;
    *o_y = canvas->viewport.y + y;
}

void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            canvas_record(canvas, x, y, r, g, b, a);
            canvas_tile_plot(ct, x, y, r, g, b, a);
            return;
        }
    SF_LIST_END();

    /*
     * It is reasonable to add a new tile only if a != 0
     */
    if (a != 0) {
        canvas_record(canvas, x, y, r, g, b, a);
        canvas_tile_plot(canvas_add_tile(canvas, x, y), x, y, r, g, b, a);
    }
}

void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
#define PTR_ASSIGN(p, v) do if (p) { *(p) = (v); } while(0)
    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            uint8_t *color;
            /* conver coordinate to canvas tile's */
            x -= ct->area.x;
            y -= ct->area.y;

            color = ct->colors + 4 * (y * ct->texture->h + x);
            PTR_ASSIGN(r, color[0]);
            PTR_ASSIGN(g, color[1]);
            PTR_ASSIGN(b, color[2]);
            PTR_ASSIGN(a, color[3]);
            return;
        }
    SF_LIST_END();

    PTR_ASSIGN(r, 0);
    PTR_ASSIGN(g, 0);
    PTR_ASSIGN(b, 0);
    PTR_ASSIGN(a, 0);
#undef PTR_ASSIGN
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

void canvas_record_begin(struct canvas *canvas) {
    struct sf_array *segment;
    if (canvas->isrecording != 0) {
        return;
    }

    canvas->isrecording = 1;

    if (canvas->cur_segment >= 0) {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(canvas->segments, canvas->cur_segment);
        if (segment->nelts == 0) {
            return;
        }
    }

    ++canvas->cur_segment;

    if (canvas->cur_segment >= canvas->segments->nelts) {
         segment = sf_array_create(sizeof(struct record),
                                   CANVAS_RECORD_NALLOC);
        sf_array_push(canvas->segments, &segment);
    } else {
        int i;
        /*
         * Make sure cur_segment is the last segment,
         * and cur_segment is empty.
         */
        for (i = canvas->cur_segment; i < canvas->segments->nelts; ++i) {
            sf_array_clear(*(struct sf_array **)
                            SF_ARRAY_NTH(canvas->segments, i), NULL);
        }
    }
}

void canvas_record_end(struct canvas *canvas) {
    canvas->isrecording = 0;
}

int canvas_record_canundo(struct canvas *canvas) {
    return canvas->cur_segment >= 0;
}

void canvas_record_undo(struct canvas *canvas) {
    struct sf_array *segment;

    if (canvas->isrecording || !canvas_record_canundo(canvas)) {
        return;
    }

    segment = *(struct sf_array **)
               SF_ARRAY_NTH(canvas->segments, canvas->cur_segment);

    SF_ARRAY_BEGIN_R(segment, struct record, record);
        /*
         * canvas->isrecoding is 0, so canvas_plot will not record.
         */
        canvas_plot(canvas, record->position.x, record->position.y,
                    record->old_color[0], record->old_color[1],
                    record->old_color[2], record->old_color[3]);
    SF_ARRAY_END();

    --canvas->cur_segment;
}

int canvas_record_canredo(struct canvas *canvas) {
    struct sf_array *segment;

    /* no next segment */
    if (canvas->cur_segment + 1 >= canvas->segments->nelts) {
        return 0;
    }

    /* only when next segment has record can we redo */
    segment = *(struct sf_array **)
               SF_ARRAY_NTH(canvas->segments, canvas->cur_segment + 1);

    return segment->nelts > 0;
}

void canvas_record_redo(struct canvas *canvas) {
    struct sf_array *segment;

    if (canvas->isrecording || !canvas_record_canredo(canvas)) {
        return;
    }

    ++canvas->cur_segment;

    segment = *(struct sf_array **)
               SF_ARRAY_NTH(canvas->segments, canvas->cur_segment);

    SF_ARRAY_BEGIN(segment, struct record, record);
        canvas_plot(canvas, record->position.x, record->position.y,
                    record->new_color[0], record->new_color[1],
                    record->new_color[2], record->new_color[3]);
    SF_ARRAY_END();
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
