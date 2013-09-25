#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"
#include "brush.h"


#define CANVAS_TILE_WIDTH 512
#define CANVAS_TILE_HEIGHT 512


/**
 * The default origin locate at up left of the viewport.
 */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    struct ivec2        offset;
    struct sf_list     *tiles;
    struct brush       *cur_brush;

    GLuint              vao, vbo;
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);


/*
 * @param x
 * @param y point at the screen coordinate.
 */
void canvas_plot(struct canvas *canvas, int x, int y,
                 float r, float g, float b, float a);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);

void canvas_set_current_brush(struct canvas *canvas, struct brush *brush);


#endif /* CANVAS_H */
