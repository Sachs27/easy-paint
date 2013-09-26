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
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);


/*
 * @param x
 * @param y point at the screen coordinate.
 */
void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);


#endif /* CANVAS_H */
