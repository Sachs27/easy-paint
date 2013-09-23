#include "sf_rect.h"


int sf_rect_iscontain(struct sf_rect *r, int x, int y) {
    int xmax, ymax;

    xmax = r->x + r->w;
    ymax = r->y + r->h;

    if (x >= r->x && x < xmax && y >= r->y && y < ymax) {
        return 1;
    }

    return 0;
}

/**
 * SAT detection.
 */
int sf_rect_isintersect(struct sf_rect *a, struct sf_rect *b) {
    int x0, x1, x2, x3;
    int y0, y1, y2, y3;

    x0 = a->x;
    x1 = a->x + a->w;
    x2 = b->x;
    x3 = b->x + b->w;

    if (x0 >= x3 || x2 >= x1) {
        return 0;
    }

    y0 = a->y;
    y1 = a->y + a->h;
    y2 = b->y;
    y3 = b->y + b->h;
    if (y0 >= y3 || y2 >= y1) {
        return 0;
    }

    return 1;
}
