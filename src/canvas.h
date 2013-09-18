#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_array.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"


struct pixel {
    struct vec2 position;
    struct vec4 color;
};

/**
 * The default origin locate at up left of the viewport.
 */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    struct ivec2        offset;
    struct sf_array    *pixels;

    GLuint              vao, vbo;

    int                 isdirty;
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);

void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      float r, float g, float b, float a);


#endif /* CANVAS_H */
