#ifndef BRUSH_H
#define BRUSH_H


#include <GL/gl.h>

#include "canvas.h"
#include "3dmath.h"


struct canvas;

struct brush {
    uint8_t color[4];

    void (*plot)(struct brush *brush, struct canvas *canvas, int x, int y);
};


void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1);

void brush_set_color(struct brush *brush,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a);

struct brush *brush_pencil_create(void);


#endif /* BRUSH_H */
