#ifndef RENDERER_H
#define RENDERER_H


#include <GL/gl.h>
#include <sf_array.h>

#include "3dmath.h"
#include "texture.h"


struct renderer2d {
    int                 w;
    int                 h;

    struct mat4         projection;

    struct sf_array    *viewports;      /* elt: (struct sf_rect)
                                         * The first elt is always the
                                         * screen's area.  */
    struct texture     *render_target;
};


/**
 * w, h are the screen's width and height
 */
struct renderer2d *renderer2d_create(int w, int h);

void renderer2d_resize(struct renderer2d *r, int w, int h);

void renderer2d_clear(struct renderer2d *renderer,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * If rt is NULL, the render target is screen.
 */
void renderer2d_set_render_target(struct renderer2d *r, struct texture *rt);

void renderer2d_draw_line(struct renderer2d *renderer, float width,
                          int x0, int y0, int x1, int y1,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Render the area (sx, sy, sw, sh) of 'texture' to the renderer2d's
 * render target's area (dx, dy, dw, dh).  If w (or h) is zero, then w (or h)
 * is assigned to the texture's (or the render target's or the viewport's if
 * render target is NULL) width (or height).
 */
void renderer2d_draw_texture(struct renderer2d *r,
                             int dx, int dy, int dw, int dh,
                             struct texture *texture,
                             int sx, int sy, int sw, int sh);

/**
 * Specify the viewport which is in the screen coordinate,
 * the y axis is from top to bottom.
 */
void renderer2d_push_viewport(struct renderer2d *r,
                              int x, int y, int w, int h);

void renderer2d_pop_viewport(struct renderer2d *r);


#endif /* RENDERER_H */
