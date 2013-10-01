#ifndef CANVAS_H
#define CANVAS_H

#include <GL/gl.h>
#include <sf_list.h>
#include <sf_array.h>

#include "sf_rect.h"
#include "3dmath.h"
#include "texture.h"
#include "brush.h"


 /* The default origin of canvas' coordinate locate at upper left of the
  * viewport, and from left to right, top to bottom.  */
struct canvas {
    struct texture     *background;
    struct sf_rect      viewport;
    float               dx, dy;
    struct sf_rect      area;
    struct sf_list     *tiles;
    int                 isrecording;
    int                 cur_segment;
    struct sf_array    *segments;       /* elt: (struct sf_array *) */
};


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h);

void canvas_draw(struct canvas *canvas);

/* Convert point (x, y) from screen's coordinate to canvas' coordinate.  */
void canvas_screen_to_canvas(struct canvas *canvas, int x, int y,
                             int *o_x, int *o_y);

/* Set the color at (x, y) where the point is in canvas' coordinate,
 * caller can use 'canvas_screen_to_canvas' to convert cooridinate.  */
void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* Get the color at (x, y) where the point is in canvas' coordinate,
 * caller can use 'canvas_screen_to_canvas' to convert cooridinate.  */
void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

void canvas_offset(struct canvas *canvas, int xoff, int yoff);

void canvas_record_begin(struct canvas *canvas);

void canvas_record_end(struct canvas *canvas);

int canvas_record_canundo(struct canvas *canvas);

void canvas_record_undo(struct canvas *canvas);

int canvas_record_canredo(struct canvas *canvas);

void canvas_record_redo(struct canvas *canvas);

void canvas_zoom_in(struct canvas *canvas, int cx, int cy);

void canvas_zoom_out(struct canvas *canvas, int cx, int cy);


#endif /* CANVAS_H */
