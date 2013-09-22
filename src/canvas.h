#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"


/**
 * The default origin locate at up left of the viewport.
 */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    struct ivec2        offset;
    struct sf_list     *tiles;

    GLuint              vao, vbo;
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);

/*
 * @param x
 * @param y point at the screen coordinate.
 */
void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      scalar_t r, scalar_t g, scalar_t b, scalar_t a);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);


#endif /* CANVAS_H */
