#ifndef BRUSH_H
#define BRUSH_H


#include <GL/gl.h>

#include "canvas.h"
#include "3dmath.h"


struct canvas;

struct brush {
    struct texture *icon;

    uint8_t color[4];

    /*
     * each brush has it's own plot function to generate different shape.
     */
    void (*plot)(struct brush *brush, struct canvas *canvas, int x, int y);
};


/**
 * Draw line from point (x0, y0) to point (x1, y1) where the points are in
 * the canvas' coordinate.
 */
void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1);

void brush_set_icon(struct brush *brush, struct texture *icon);

void brush_set_color(struct brush *brush,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a);

struct brush *brush_pencil_create(void);

struct brush *brush_eraser_create(void);

struct brush *brush_pen_create(void);


#endif /* BRUSH_H */
