#include <stdlib.h>
#include <math.h>

#include <sf_utils.h>

#include "brush.h"


/**
 * Bresenham's line algorithm. Keep the origin line orientation.
 */
void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1) {
    int x, y;
    int dx, dy;
    int dx2, dy2;
    int xstep, ystep;
    int err;

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
        for (x = x0; x != x1; x += xstep) {
            /* plot(px, py) */
            brush->plot(brush, canvas, x, y);

            if (err > 0) {
                err -= dx2;          /* e = e - 1 */
                y += ystep;
            }

            err += dy2;
        }
    } else {
        err = dx2 - dy;
        for (y = y0; y != y1; y += ystep) {
            brush->plot(brush, canvas, x, y);

            if (err >= 0) {
                err -= dy2;
                x += xstep;
            }

            err += dx2;
        }
    }
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

