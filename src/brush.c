#include <stdlib.h>
#include <math.h>

#include "brush.h"

/**
 * Bresenham's line algorithm.
 */
void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1) {
#define int_swap(x, y) do {     \
    int __int_swap_tmp__ = x;   \
    x = y;                      \
    y = __int_swap_tmp__;       \
} while(0)

    int err;
    int x, y, ystep;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int steep = (dy - dx) > 0 ? 1 : 0;

    if (brush->plot == NULL) {
        return;
    }

    if (steep) {
        int_swap(x0, y0);
        int_swap(x1, y1);
    }

    if (x0 > x1) {
        int_swap(x0, x1);
        int_swap(y0, y1);
    }

    dx = x1 - x0;
    dy = abs(y1 - y0);
    err = dx / 2;
    y = y0;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = x0; x <= x1; ++x) {
        int px = x, py = y;
        if (steep) {
            px = y;
            py = x;
        }
        /* plot(px, py) */
        brush->plot(brush, canvas, px, py);

        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
#undef int_swap
}

void brush_set_icon(struct brush *brush, struct texture *icon) {
    brush->icon = icon;
}

void brush_set_color(struct brush *brush,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    brush->color[0] = r;
    brush->color[1] = g;
    brush->color[2] = b;
    brush->color[3] = a;
}

