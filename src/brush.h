#ifndef BRUSH_H
#define BRUSH_H


#include <inttypes.h>

struct canvas;

enum BRUSH_TYPE {
    BRUSH_PEN = 1,
    BRUSH_PENCIL,
    BRUSH_ERASER,
};

struct brush {
    int   blend_mode;
    float color[4];
    float radius;
};


int brush_init(struct brush *brush, int type);

/**
 * Draw line from point (x0, y0) to point (x1, y1) where the points are in
 * the canvas' coordinate.
 */
void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1);

void brush_set_color(struct brush *brush, float r, float g, float b, float a);

int brush_cmp(struct brush *b0, struct brush *b1);

struct brush *brush_pencil_create(void);

struct brush *brush_eraser_create(void);

struct brush *brush_pen_create(void);


#endif /* BRUSH_H */
