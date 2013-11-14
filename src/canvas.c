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
    ct->area.x = x;
    ct->area.y = y;
    ct->area.w = ct->texture->w;
    ct->area.h = ct->texture->h;
    ct->isdirty = 1;
    ct->dirty_rect.x = 0;
    ct->dirty_rect.y = 0;
    ct->dirty_rect.w = ct->area.w;
    ct->dirty_rect.h = ct->area.h;
    ct->colors = sf_calloc(ct->texture->w * ct->texture->h * 4 * sizeof(uint8_t));
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
    uint32_t *colors; /*[ct->dirty_rect.w * ct->dirty_rect.h];*/
    uint32_t *row;
    uint32_t *src_row;
    int       i;

    src_row = ((uint32_t *) (ct->colors))
                                + ct->dirty_rect.y * ct->texture->w
                                + ct->dirty_rect.x;
    colors = sf_alloc(ct->dirty_rect.w * ct->dirty_rect.h * sizeof(uint32_t));
    row = colors;

    for (i = 0; i < ct->dirty_rect.h; ++i) {
        memcpy(row, src_row, ct->dirty_rect.w * sizeof(uint32_t));
        row += ct->dirty_rect.w;
        src_row += ct->texture->w;
    }
    /* use dirty rect */
    glBindTexture(GL_TEXTURE_2D, ct->texture->tid);

    /* On some Android device, glTexSubImage2D don't work, why? */
#ifdef ANDROID
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ct->texture->w, ct->texture->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, ct->colors);
#else
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    ct->dirty_rect.x, ct->dirty_rect.y,
                    ct->dirty_rect.w, ct->dirty_rect.h,
                    GL_RGBA, GL_UNSIGNED_BYTE, colors);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);
    ct->isdirty = 0;
    sf_free(colors);
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

    sf_log(SF_LOG_INFO, "canvas has %d tiles\n",
           sf_list_cnt(&canvas->tiles) + 1);

    sf_list_push(&canvas->tiles, &ct);

    return sf_list_tail(&canvas->tiles);
}

static void canvas_on_render(struct canvas *canvas,
                             struct renderer2d *renderer2d) {
    sf_list_iter_t iter;
    float scale = canvas->viewport.w * 1.0 / canvas->ui.area.w;
    /* draw background */
    renderer2d_clear(renderer2d, 255, 255, 255, 0);
    /* draw pixels */
    if (sf_list_begin(&canvas->tiles, &iter)) do {
        struct canvas_tile *ct = sf_list_iter_elt(&iter);
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
    } while (sf_list_iter_next(&iter));
}

static void canvas_on_resize(struct canvas *canvas, int w, int h) {
    /*float scale = ((float) canvas->viewport.w) / canvas->ui.area.w;*/

    canvas->ui.area.w = w;
    canvas->ui.area.h = h;

    canvas->viewport.w = canvas->ui.area.w;
    canvas->viewport.h = canvas->ui.area.h;
}


struct canvas *canvas_create(int w, int h) {
    struct canvas *canvas;

    canvas = sf_alloc(sizeof(*canvas));
    canvas_init(canvas, w, h);
    return canvas;
}

int canvas_init(struct canvas *canvas, int w, int h) {
    sf_list_def_t def;

    ui_init((struct ui *) canvas, w, h);

    canvas->background = NULL;
    canvas->viewport.x = 0;
    canvas->viewport.y = 0;
    canvas->viewport.w = w;
    canvas->viewport.h = h;
    canvas->dx = canvas->dy = 0.0f;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct canvas_tile);
    if (sf_list_init(&canvas->tiles, &def) != SF_OK) {
        return -1;
    }
    canvas_add_tile(canvas, 0, 0);
    canvas->record = NULL;

    UI_CALLBACK(canvas, render, canvas_on_render);
    UI_CALLBACK(canvas, resize, canvas_on_resize);

    return 0;
}

void canvas_clear(struct canvas *canvas) {
    sf_list_iter_t iter;

    if (sf_list_begin(&canvas->tiles, &iter)) do {
        struct canvas_tile *ct = sf_list_iter_elt(&iter);

        memset(ct->colors, 0,
               ct->texture->w * ct->texture->h * 4 * sizeof(uint8_t));

        ct->isdirty = 1;
        ct->dirty_rect.x = 0;
        ct->dirty_rect.y = 0;
        ct->dirty_rect.w = ct->texture->w;
        ct->dirty_rect.h = ct->texture->h;
    } while (sf_list_iter_next(&iter));
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

void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    sf_list_iter_t  iter;
    uint8_t         ocolor[4];

    if (sf_list_begin(&canvas->tiles, &iter)) do {
        struct canvas_tile *ct = sf_list_iter_elt(&iter);

        if (sf_rect_iscontain(&ct->area, x, y)) {
            if (canvas->record) {
                canvas_pick(canvas, x, y,
                            ocolor, ocolor + 1, ocolor + 2, ocolor + 3);
                record_record(canvas->record, x, y,
                              ocolor[0], ocolor[1], ocolor[2], ocolor[3],
                              r, g, b, a);
            }
            canvas_tile_plot(ct, x, y, r, g, b, a);
            return;
        }
    } while (sf_list_iter_next(&iter));

    /*
     * It is reasonable to add a new tile only if a != 0
     */
    if (a != 0) {
        if (canvas->record) {
            canvas_pick(canvas, x, y,
                        ocolor, ocolor + 1, ocolor + 2, ocolor + 3);
            record_record(canvas->record, x, y,
                          ocolor[0], ocolor[1], ocolor[2], ocolor[3],
                          r, g, b, a);
        }
        canvas_tile_plot(canvas_add_tile(canvas, x, y), x, y, r, g, b, a);
    }
}

void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
    sf_list_iter_t iter;

#define PTR_ASSIGN(p, v) do if (p) { *(p) = (v); } while(0)
    if (sf_list_begin(&canvas->tiles, &iter)) do {
        struct canvas_tile *ct = sf_list_iter_elt(&iter);

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
    } while (sf_list_iter_next(&iter));

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
