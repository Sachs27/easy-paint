#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"
#include "brush.h"


/**
 * The default origin locate at up left of the viewport.
 */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    struct ivec2        offset;

    struct texture     *texture;
    int                 isdirty;
    struct sf_array    *dirty_pixels;

    struct brush       *cur_brush;
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);


/*
 * @param x
 * @param y point at the screen coordinate.
 */
void canvas_plot(struct canvas *canvas, int x, int y);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);

void canvas_set_current_brush(struct canvas *canvas, struct brush *brush);


#endif /* CANVAS_H */
