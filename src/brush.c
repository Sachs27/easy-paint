#include <stdlib.h>
#include <math.h>

#include <sf/array.h>
#include <sf/log.h>
#include <sf/utils.h>

#include "sf_rect.h"
#include "brush.h"
#include "canvas.h"

#if 0
static struct sf_rect calc_area(int x0, int y0, int x1, int y1, int r) {
    struct sf_rect area;

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

    return area;
}


static void blend_eraser(struct canvas *canvas, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
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

static void blend_normal(struct canvas *canvas, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    float   src[4];
    float   dst[4];
    float   out[4];
    uint8_t picked[4];
    uint8_t plot[4];

    if (a == 0) {
        return;
    }

    src[0] = r / 255.0f;
    src[1] = g / 255.0f;
    src[2] = b / 255.0f;
    src[3] = a / 255.0f;

    canvas_pick(canvas, x, y, picked, picked + 1, picked + 2, picked + 3);
    dst[0] = picked[0] / 255.0f;
    dst[1] = picked[1] / 255.0f;
    dst[2] = picked[2] / 255.0f;
    dst[3] = picked[3] / 255.0f;

    /* out[3] > 0 */
    out[3] = dst[3] * (1 - src[3]) + src[3];
    out[0] = (dst[0] * dst[3] * (1 - src[3]) + src[0] * src[3]) / out[3];
    out[1] = (dst[1] * dst[3] * (1 - src[3]) + src[1] * src[3]) / out[3];
    out[2] = (dst[2] * dst[3] * (1 - src[3]) + src[2] * src[3]) / out[3];

    plot[0] = out[0] * 255;
    plot[1] = out[1] * 255;
    plot[2] = out[2] * 255;
    plot[3] = out[3] * 255;
    /* no need to plot the same color */
    if (plot[0] == picked[0] && plot[1] == picked[1]
        && plot[2] == picked[2] && plot[3] == picked[3]) {
        return;
    }

    canvas_plot(canvas, x, y, plot[0], plot[1], plot[2], plot[3]);
}

static void blend_rnormal(struct canvas *canvas, int x, int y,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    float   src[4];
    float   dst[4];
    float   out[4];
    uint8_t picked[4];
    uint8_t plot[4];

    if (a == 0) {
        return;
    }

    src[0] = r / 255.0f;
    src[1] = g / 255.0f;
    src[2] = b / 255.0f;
    src[3] = a / 255.0f;

    canvas_pick(canvas, x, y, picked, picked + 1, picked + 2, picked + 3);
    out[0] = picked[0] / 255.0f;
    out[1] = picked[1] / 255.0f;
    out[2] = picked[2] / 255.0f;
    out[3] = picked[3] / 255.0f;

    dst[3] = (out[3] - src[3]) / (1 - src[3]);
    dst[0] = (out[0] * out[3] - src[0] * src[3]) / ((1 - src[3]) * dst[3]);
    dst[1] = (out[1] * out[3] - src[1] * src[3]) / ((1 - src[3]) * dst[3]);
    dst[2] = (out[2] * out[3] - src[2] * src[3]) / ((1 - src[3]) * dst[3]);

    plot[0] = dst[0] * 255;
    plot[1] = dst[1] * 255;
    plot[2] = dst[2] * 255;
    plot[3] = dst[3] * 255;
    /* no need to plot the same color */
    if (plot[0] == picked[0] && plot[1] == picked[1]
        && plot[2] == picked[2] && plot[3] == picked[3]) {
        return;
    }

    canvas_plot(canvas, x, y, plot[0], plot[1], plot[2], plot[3]);
}
#endif
#if 0
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

        /* outline */
        blend_alpha(canvas, xc + x, yc + y, alpha);
        blend_alpha(canvas, xc - x, yc + y, alpha);
        blend_alpha(canvas, xc + x, yc - y, alpha);
        blend_alpha(canvas, xc - x, yc - y, alpha);
        blend_alpha(canvas, xc + y, yc + x, alpha);
        blend_alpha(canvas, xc - y, yc + x, alpha);
        blend_alpha(canvas, xc + y, yc - x, alpha);
        blend_alpha(canvas, xc - y, yc - x, alpha);

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
#endif
#if 0
static void plot_point(struct canvas *canvas, struct brush *brush,
                       struct sf_rect area, int xc, int yc) {
    brush->blend(canvas, xc, yc, brush->color[0], brush->color[1],
          brush->color[2], brush->color[3]);
}

static void plot_circle(struct canvas *canvas, struct brush *brush,
                        struct sf_rect area, int xc, int yc) {
    int r2 = brush->radius * brush->radius;
    int x, y;

    /*sf_log(SF_LOG_DEBUG, "plot_circle at (%d, %d).", xc, yc);*/

    for (x = area.x; x < area.x + area.w; ++x)
    for (y = area.y; y < area.y + area.h; ++y) {
        int dx = x - xc;
        int dy = y - yc;
        int d2 = dx * dx + dy * dy;
        if (d2 < r2) {
            float t = 1.0f * d2 / r2;
            uint8_t a;

            a = brush->color[3];
            brush->blend(canvas, x, y, brush->color[0], brush->color[1],
                         brush->color[2], a);
        }
    }
}
#endif
static int brush_eraser_init(struct brush *eraser) {
    eraser->radius = 4;
    brush_set_color(eraser, 0, 0, 0, 128);
    return 0;
}

static int brush_pencil_init(struct brush *pencil) {
    pencil->radius = 2;
    brush_set_color(pencil, 0, 0, 0, 128);
    return 0;
}

static int brush_pen_init(struct brush *pen) {
    pen->radius = 8;
    /* max alpha = 128 */
    brush_set_color(pen, 1, 0, 0, 0.5);
    return 0;
}

int brush_init(struct brush *brush, int type) {
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
    int x, y;
    int dx, dy;
    int dx2, dy2;
    int step;
    int xstep, ystep;
    int err;

    canvas_set_plot_color(canvas, brush->color);
    canvas_set_plot_size(canvas, brush->radius);

    dx = x1 - x0;
    dy = y1 - y0;

    step = 1;
    if (dx > 0) {
        xstep = step;
    } else {
        xstep = -step;
        dx = -dx;
    }

    if (dy > 0) {
        ystep = step;
    } else {
        ystep = -step;
        dy = -dy;
    }

    dx2 = dx << 1;
    dy2 = dy << 1;

    x = x0;
    y = y0;

    if (dx > dy) {
        err = dy2 - dx;              /* e = dy / dx - 0.5 */
        for (x = x0; xstep > 0 ? x <= x1 : x >= x1; x += xstep) {
            /* plot(px, py) */
            canvas_plot(canvas, x, y);

            if (err > 0) {
                err -= dx2;          /* e = e - 1 */
                y += ystep;
            }

            err += dy2;
        }
    } else {
        err = dx2 - dy;
        for (y = y0; ystep > 0 ? y <= y1 : y >= y1; y += ystep) {
            canvas_plot(canvas, x, y);

            if (err >= 0) {
                err -= dy2;
                x += xstep;
            }

            err += dx2;
        }
    }
}

void brush_set_color(struct brush *brush, float r, float g, float b, float a) {
    brush->color[0] = r;
    brush->color[1] = g;
    brush->color[2] = b;
    brush->color[3] = a;
}
