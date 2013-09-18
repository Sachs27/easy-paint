#include "sf_rect.h"


int sf_rect_iscontain(struct sf_rect *r, int x, int y) {
    int xmax, ymax;

    xmax = r->x + r->w;
    ymax = r->y + r->h;

    if (x > r->x && x < xmax && y > r->y && y < ymax) {
        return 1;
    }

    return 0;
}

