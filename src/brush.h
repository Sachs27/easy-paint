#ifndef BRUSH_H
#define BRUSH_H


#include "3dmath.h"


struct brush {
    struct vec4 color;

    void (*plot)(struct brush *brush, int x, int y);
};


void brush_drawline(struct brush *brush, int x0, int y0, int x1, int y1);

void brush_set_color(struct brush *brush,
                     scalar_t r, scalar_t g, scalar_t b, scalar_t a);


struct brush *brush_pencil_create(void);


#endif /* BRUSH_H */
