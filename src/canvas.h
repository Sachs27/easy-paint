#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"


struct canvas_tile {
    struct sf_rect area;

    int isdirty;
    struct sf_rect dirty_rect;

    struct texture *texture;
    struct vec4 *colors;
};

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

void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      scalar_t r, scalar_t g, scalar_t b, scalar_t a);


#endif /* CANVAS_H */
