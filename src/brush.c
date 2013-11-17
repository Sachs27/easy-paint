#include <math.h>

#include <sf/array.h>
#include <sf/utils.h>

#include "sf_rect.h"
#include "brush.h"
#include "canvas.h"


struct brush_buf {
    struct sf_rect  area;
    sf_array_t      pixels;     /* elt: uint32_t */
} *brush_buf = NULL;


int brush_buf_init(struct brush_buf *bb) {
    sf_array_def_t def;

    sf_memzero(&bb->area, sizeof(bb->area));

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(uint32_t);
    return sf_array_init(&bb->pixels, &def);
}

void brush_buf_set_area(struct brush_buf *bb, int x0, int y0,
                        int x1, int y1, uint32_t r) {
    struct sf_rect  area;

    if (x1 < x0) {
        sf_swap(int, x1, x0);
    }

    if (y1 < y0) {
        sf_swap(int, y1, y0);
    }

    x0 -= r;
    y0 -= r;
    x1 += r;
    y1 += r;

    area.x = x0;
    area.y = y0;
    area.w = x1 - x0 + 1;
    area.h = y1 - y0 + 1;

    sf_array_clear(&bb->pixels);

    sf_array_alloc(&bb->pixels, area.w * area.h);

    bb->area = area;
}

void brush_buf_plot(struct brush_buf *bb, int x, int y,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint8_t *color;

    x -= bb->area.x;
    y -= bb->area.y;

    if (x < 0 || x >= bb->area.w || y < 0 || y >= bb->area.h) {
        return;
    }

    color = (uint8_t *) sf_array_nth(&bb->pixels, y * bb->area.w + x);

    if (a > color[3]) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
    }
}

static void blend_alpha(struct canvas *canvas, int x, int y, uint8_t a) {
    uint8_t dst_color[4];
    uint8_t alpha;

    canvas_pick(canvas, x, y, dst_color, dst_color + 1, dst_color + 2,
                dst_color + 3);

    alpha = dst_color[3] - a < 0 ? 0 : dst_color[3] - a;

    /* no need to plot the same color */
    if (alpha == dst_color[3]) {
        return;
    }

    canvas_plot(canvas, x, y, dst_color[0], dst_color[1], dst_color[2],
                alpha);
}

static void blend_additive(struct canvas *canvas, int x, int y,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#define CLAMP(i, min, max) do { \
    if (a < min) {              \
        a = min;                \
    } else if (a > max) {       \
        a = max;                \
    }                           \
} while (0)
    float   alpha;
    uint8_t dst_color[4];
    uint8_t color[4];

    if (a == 0) {
        return;
    }

    canvas_pick(canvas, x, y, dst_color, dst_color + 1, dst_color + 2,
                dst_color + 3);
#if 0
    dst_color[0] = 255 - dst_color[0];
    dst_color[1] = 255 - dst_color[1];
    dst_color[2] = 255 - dst_color[2];
    dst_color[3] = 255 - dst_color[3];

    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
#endif

    alpha = a / 255.0f;
    color[0] = dst_color[0] * (1 - alpha) + r * alpha;
    color[1] = dst_color[1] * (1 - alpha) + g * alpha;
    color[2] = dst_color[2] * (1 - alpha) + b * alpha;
    float aaaa = dst_color[3] / 255.0f;
    alpha = alpha * a + dst_color[3];
    color[3] = alpha >= 255.0f ? (uint8_t) 255 : (uint8_t) alpha;
    /*color[3] = dst_color[3] * (1 - alpha) + a * alpha + a;*/

    /* no need to plot the same color */
    if (color[0] == dst_color[0] && color[1] == dst_color[1]
        && color[2] == dst_color[2] && color[3] == dst_color[3]) {
        return;
    }
#if 0
    canvas_plot(canvas, x, y, 255 - color[0], 255 - color[1],
                255 - color[2], 255 - color[3]);
#endif
    canvas_plot(canvas, x, y, color[0], color[1], color[2], color[3]);
}

static void pen_plot(struct brush *brush, struct canvas *canvas,
                           int px, int py) {
    /*
     * z x z
     * x o x
     * z x z
     */
    uint8_t r = brush->color[0];
    uint8_t g = brush->color[1];
    uint8_t b = brush->color[2];
    uint8_t a0 = brush->color[3];
    uint8_t a1 = a0 * 0.6;
    uint8_t a2 = a0 * 0.3;
#if 0
    blend_additive(canvas, px - 1, py - 1, r, g, b, a2);
    blend_additive(canvas, px, py - 1, r, g, b, a1);
    blend_additive(canvas, px + 1, py - 1, r, g, b, a2);
    blend_additive(canvas, px - 1, py, r, g, b, a1);
    blend_additive(canvas, px, py, r, g, b, a0);
    blend_additive(canvas, px + 1, py, r, g, b, a1);
    blend_additive(canvas, px - 1, py + 1, r, g, b, a2);
    blend_additive(canvas, px, py + 1, r, g, b, a1);
    blend_additive(canvas, px + 1, py + 1, r, g, b, a2);
#endif
    const int radius = 3;
    int xc = px, yc = py;
    int x = 0, y = radius, i, d;
    int lastx = -1, lasty = -1;

    d = 3 - 2 * radius;

    /* fill the cycle */
    while (x <= y) {
        /*uint8_t alpha = 255;*/

        if (lasty != y) {
            lasty = y;
            for (i = -x + 1; i < x; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + y * y) / r;*/
                blend_additive(canvas, xc + i, yc + y, r, g, b, a0);
                if (yc - y != yc + y) {
                    blend_additive(canvas, xc + i, yc - y, r, g, b, a0);
                }
            }
        }
        if (lastx != x) {
            lastx = x;
            for (i = -y + 1; i < y; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + x * x) / r;*/
                blend_additive(canvas, xc + i, yc + x, r, g, b, a0);
                if (yc - x != yc + x) {
                    blend_additive(canvas, xc + i, yc - x, r, g, b, a0);
                }
            }
        }

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            --y;
        }
        ++x;
    }
}

static void pencil_plot(struct brush *brush, struct canvas *canvas,
                        int px, int py) {
    uint8_t r = brush->color[0];
    uint8_t g = brush->color[1];
    uint8_t b = brush->color[2];
    uint8_t a0 = brush->color[3];
    uint8_t a1 = (a0 / 255.0f) * (a0 / 255.0f) * 255;
    uint8_t a2 = (a1 / 255.0f) * (a1 / 255.0f) * 255;

    switch (rand() % 5) {
    case 0:
        /*
         *  o x
         *  x x
         */
        blend_additive(canvas, px, py, r, g, b, a0);
        blend_additive(canvas, px + 1, py, r, g, b, a1);
        blend_additive(canvas, px, py + 1, r, g, b, a1);
        blend_additive(canvas, px + 1, py + 1, r, g, b, a2);
        break;
    case 1:
        /*
         * x o
         * x x
         */
        blend_additive(canvas, px, py, r, g, b, a1);
        blend_additive(canvas, px + 1, py, r, g, b, a0);
        blend_additive(canvas, px, py + 1, r, g, b, a2);
        blend_additive(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 2:
        /*
         * x x
         * o x
         */
        blend_additive(canvas, px, py, r, g, b, a1);
        blend_additive(canvas, px + 1, py, r, g, b, a2);
        blend_additive(canvas, px, py + 1, r, g, b, a0);
        blend_additive(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 3:
        /*
         * x x
         * x o
         */
        blend_additive(canvas, px, py, r, g, b, a2);
        blend_additive(canvas, px + 1, py, r, g, b, a1);
        blend_additive(canvas, px, py + 1, r, g, b, a1);
        blend_additive(canvas, px + 1, py + 1, r, g, b, a0);
        break;
    case 4:
        /*
         *   x
         * x o x
         *   x
         */
        blend_additive(canvas, px, py - 1, r, g, b, a1);
        blend_additive(canvas, px - 1, py, r, g, b, a1);
        blend_additive(canvas, px, py, r, g, b, a0);
        blend_additive(canvas, px + 1, py, r, g, b, a1);
        blend_additive(canvas, px, py + 1, r, g, b, a1);
        break;
    }
}

/**
 * Bresenham's circle algorithm.
 */
static void eraser_plot(struct brush *brush, struct canvas *canvas,
                        int xc, int yc) {
    const int r = 8;
    int x = 0, y = r, i, d;
    int lastx = -1, lasty = -1;

    d = 3 - 2 * r;

    /* fill the cycle */
    while (x <= y) {
        uint8_t alpha = 255;

#if 0
        /* outline */
        blend_alpha(canvas, xc + x, yc + y, alpha);
        blend_alpha(canvas, xc - x, yc + y, alpha);
        blend_alpha(canvas, xc + x, yc - y, alpha);
        blend_alpha(canvas, xc - x, yc - y, alpha);
        blend_alpha(canvas, xc + y, yc + x, alpha);
        blend_alpha(canvas, xc - y, yc + x, alpha);
        blend_alpha(canvas, xc + y, yc - x, alpha);
        blend_alpha(canvas, xc - y, yc - x, alpha);
#endif
        if (lasty != y) {
            lasty = y;
            for (i = -x + 1; i < x; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + y * y) / r;*/
                blend_alpha(canvas, xc + i, yc + y, alpha);
                if (yc - y != yc + y) {
                    blend_alpha(canvas, xc + i, yc - y, alpha);
                }
            }
        }
        if (lastx != x) {
            lastx = x;
            for (i = -y + 1; i < y; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + x * x) / r;*/
                blend_alpha(canvas, xc + i, yc + x, alpha);
                if (yc - x != yc + x) {
                    blend_alpha(canvas, xc + i, yc - x, alpha);
                }
            }
        }

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            --y;
        }
        ++x;
    }
}

static void plot_point(struct brush *brush, struct brush_buf *bb,
                       int xc, int yc) {
    brush_buf_plot(bb, xc, yc, brush->color[0], brush->color[1],
                   brush->color[2], brush->color[3]);
}

static void plot_circle(struct canvas * canvas, struct brush *brush,
                        struct brush_buf *bb,
                        int xc, int yc) {
    int r = 7;
    int r2 = brush->radius * brush->radius;
    int x, y;

    for (x = bb->area.x; x < bb->area.x + bb->area.w; ++x)
    for (y = bb->area.y; y < bb->area.y + bb->area.h; ++y) {
        int dx = x - xc;
        int dy = y - yc;
        int d2 = dx * dx + dy * dy;
        if (d2 < r2) {
            float t = 1.0f * d2 / r2;
            uint8_t a;

            a = (1 - t) * brush->color[3];
            /*blend_additive(canvas, x, y, brush->color[0], brush->color[1],*/
                           /*brush->color[2], a);*/
            brush_buf_plot(bb, x, y, brush->color[0],
                           brush->color[1], brush->color[2], a);
        }
    }
}

static int brush_eraser_init(struct brush *eraser) {
    eraser->plot = eraser_plot;
    brush_set_color(eraser, 0, 0, 0, 255);
    return 0;
}

static int brush_pencil_init(struct brush *pencil) {
    pencil->plot = pencil_plot;
    brush_set_color(pencil, 0, 0, 0, 200);
    return 0;
}

static int brush_pen_init(struct brush *pen) {
    pen->plot = pen_plot;
    brush_set_color(pen, 128, 0, 0, 64);
    return 0;
}

int brush_init(struct brush *brush, int type) {
    brush->radius = 16;
    switch (type) {
    case BRUSH_PEN:
        return brush_pen_init(brush);
    case BRUSH_PENCIL:
        return brush_pencil_init(brush);
    case BRUSH_ERASER:
        return brush_eraser_init(brush);
    }
    return -1;
}

/**
 * Bresenham's line algorithm. Keep the origin line orientation.
 */
void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1) {
    sf_array_iter_t iter;
    int x, y;
    int dx, dy;
    int dx2, dy2;
    int xstep, ystep;
    int err;

    if (brush_buf == NULL) {
        brush_buf = sf_alloc(sizeof(*brush_buf));
        brush_buf_init(brush_buf);
    }

    brush_buf_set_area(brush_buf, x0, y0, x1, y1, brush->radius);

    dx = x1 - x0;
    dy = y1 - y0;

    if (dx > 0) {
        xstep = 1;
    } else {
        xstep = -1;
        dx = -dx;
    }

    if (dy > 0) {
        ystep = 1;
    } else {
        ystep = -1;
        dy = -dy;
    }

    dx2 = dx << 1;
    dy2 = dy << 1;

    x = x0;
    y = y0;

    if (dx > dy) {
        err = dy2 - dx;              /* e = dy / dx - 0.5 */
        for (x = x0; xstep > 0 ? x < x1 : x > x1; x += xstep) {
            /* plot(px, py) */
            /*brush->plot(brush, canvas, x, y);*/
            plot_circle(canvas, brush, brush_buf, x, y);

            if (err > 0) {
                err -= dx2;          /* e = e - 1 */
                y += ystep;
            }

            err += dy2;
        }
    } else {
        err = dx2 - dy;
        for (y = y0; ystep > 0 ? y < y1 : y > y1; y += ystep) {
            /*brush->plot(brush, canvas, x, y);*/
            plot_circle(canvas, brush, brush_buf, x, y);

            if (err >= 0) {
                err -= dy2;
                x += xstep;
            }

            err += dx2;
        }
    }

    if (x = 0, y = 0, sf_array_begin(&brush_buf->pixels, &iter)) do {
        uint8_t *color = sf_array_iter_elt(&iter);

        blend_additive(canvas, x + brush_buf->area.x, y + brush_buf->area.y,
                       color[0], color[1], color[2], color[3]);
        ++x;
        if (x >= brush_buf->area.w) {
            ++y;
            x = 0;
        }
    } while (sf_array_iter_next(&iter));

}

void brush_set_color(struct brush *brush,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    brush->color[0] = r;
    brush->color[1] = g;
    brush->color[2] = b;
    brush->color[3] = a;
}
