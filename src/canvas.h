#ifndef CANVAS_H
#define CANVAS_H


#include <sf/defs.h>
#include <sf/array.h>

#include "ui.h"
#include "texture.h"

struct renderer2d;

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
/* ---------------------------------------------*/
    struct renderer2d  *renderer;
    sf_bool_t           isploting;
    sf_bool_t           iscontent_inited;
    struct texture      content;
    sf_bool_t           isbuffet_inited;
    struct texture      buffer;

    float               plot_color[4];
    float               plot_size;
    sf_array_t          plots;          /* elts: (struct vec2) */
};


int canvas_init(struct canvas *canvas, int w, int h);

void canvas_destroy(struct canvas *canvas);

void canvas_clear(struct canvas *canvas);

/**
 * Convert point (x, y) from screen's coordinate to canvas' coordinate.
 */
void canvas_screen_to_canvas(struct canvas *canvas, int x, int y,
                             int *o_x, int *o_y);

void canvas_begin_plot(struct canvas *canvas);

/**
 * Plot at (x, y) where the point is in canvas' coordinate,
 * caller can use 'canvas_screen_to_canvas' to convert cooridinate.
 */
void canvas_plot(struct canvas *canvas, float x, float y);

void canvas_end_plot(struct canvas *canvas);

void canvas_set_plot_color(struct canvas *canvas, float color[4]);

void canvas_set_plot_size(struct canvas *canvas, float size);

#endif /* CANVAS_H */
