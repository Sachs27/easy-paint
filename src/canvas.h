#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>
#include <sf_array.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"
#include "brush.h"


/**
 * The default origin locate at upper left of the viewport.
 */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    struct ivec2        offset;
    struct sf_list     *tiles;

    int                 isrecording;
    int                 cur_segment;
    struct sf_array    *segments;       /* elt: (struct sf_array *) */
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

void canvas_record_begin(struct canvas *canvas);

void canvas_record_end(struct canvas *canvas);

void canvas_record_undo(struct canvas *canvas);


#endif /* CANVAS_H */
