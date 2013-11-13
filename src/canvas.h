#ifndef CANVAS_H
#define CANVAS_H

#include <sf/list.h>

#include "ui.h"


struct texture;
struct record;

 /**
  * The default origin of canvas' coordinate locate at upper left of the
  * viewport, and from left to right, top to bottom.
  */
struct canvas {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */

    struct texture     *background;

    struct sf_rect      viewport;       /* the viewport of camera in the
                                         * canvas's coordinate  */

    float               dx, dy;         /* the delta of offset */

    sf_list_t           tiles;          /* elt: (struct canvas_tile) */

    struct record      *record;
};


struct canvas *canvas_create(int w, int h);

int canvas_init(struct canvas *canvas, int w, int h);

void canvas_clear(struct canvas *canvas);

/**
 * Convert point (x, y) from screen's coordinate to canvas' coordinate.
 */
void canvas_screen_to_canvas(struct canvas *canvas, int x, int y,
                             int *o_x, int *o_y);

/**
 * Set the color at (x, y) where the point is in canvas' coordinate,
 * caller can use 'canvas_screen_to_canvas' to convert cooridinate.
 */
void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Get the color at (x, y) where the point is in canvas' coordinate,
 * caller can use 'canvas_screen_to_canvas' to convert cooridinate.
 */
void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);

void canvas_zoom_in(struct canvas *canvas, int cx, int cy);

void canvas_zoom_out(struct canvas *canvas, int cx, int cy);


#endif /* CANVAS_H */
