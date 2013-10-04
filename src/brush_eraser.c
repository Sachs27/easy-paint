#include <stdlib.h>
#include <math.h>

#include "brush.h"


static void eraser_blend(struct canvas *canvas, int x, int y, uint8_t a) {
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

/* Bresenham's circle algorithm */
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
        eraser_blend(canvas, xc + x, yc + y, alpha);
        eraser_blend(canvas, xc - x, yc + y, alpha);
        eraser_blend(canvas, xc + x, yc - y, alpha);
        eraser_blend(canvas, xc - x, yc - y, alpha);
        eraser_blend(canvas, xc + y, yc + x, alpha);
        eraser_blend(canvas, xc - y, yc + x, alpha);
        eraser_blend(canvas, xc + y, yc - x, alpha);
        eraser_blend(canvas, xc - y, yc - x, alpha);
#endif
        /* fill the cycle */
        if (lasty != y) {
            lasty = y;
            for (i = -x + 1; i < x; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + y * y) / r;*/
                eraser_blend(canvas, xc + i, yc + y, alpha);
                if (yc - y != yc + y) {
                    eraser_blend(canvas, xc + i, yc - y, alpha);
                }
            }
        }
        if (lastx != x) {
            lastx = x;
            for (i = -y + 1; i < y; ++i) {
                /*alpha = 255 - 255 * sqrt(i * i + x * x) / r;*/
                eraser_blend(canvas, xc + i, yc + x, alpha);
                if (yc - x != yc + x) {
                    eraser_blend(canvas, xc + i, yc - x, alpha);
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


struct brush *brush_eraser_create(void) {
    struct brush *eraser;

    eraser = malloc(sizeof(*eraser));
    eraser->plot = eraser_plot;
    brush_set_color(eraser, 0, 0, 0, 255);
    brush_set_icon(eraser, texture_load_2d("res/icons/eraser.png"));

    return eraser;
}
