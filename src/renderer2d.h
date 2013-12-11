#ifndef RENDERER_H
#define RENDERER_H


#ifdef GLES2
# include <GLES2/gl2.h>
#else
# include <GL/gl.h>
#endif

#include <sf/array.h>

#include "3dmath.h"
#include "texture.h"


/**
 * w, h are the screen's width and height
 */
int renderer2d_init(int w, int h);

void renderer2d_destroy(void);

void renderer2d_resize(int w, int h);

void renderer2d_clear(float r, float g, float b, float a);

void renderer2d_blend_points(struct texture *dst, struct vec2 *points,
                             size_t npoints, float size,
                             float r, float g, float b, float a);

/**
 * If rt is NULL, the render target is screen.
 */
void renderer2d_set_render_target(struct texture *rt);

void renderer2d_fill_rect(int x, int y, int w, int h,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void renderer2d_draw_line(float width, int x0, int y0, int x1, int y1,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Render the area (sx, sy, sw, sh) of 'texture' to the renderer2d's
 * render target's area (dx, dy, dw, dh).  If w (or h) is zero, then w (or h)
 * is assigned to the texture's (or the render target's or the viewport's if
 * render target is NULL) width (or height).
 */
void renderer2d_draw_texture(int dx, int dy, int dw, int dh,
                             struct texture *texture,
                             int sx, int sy, int sw, int sh);

void renderer2d_draw_texture_with_color(int dx, int dy, int dw, int dh,
                                        struct texture *texture,
                                        int sx, int sy, int sw, int sh,
                                        uint8_t r, uint8_t g, uint8_t b);


/**
 * Specify the viewport which is in the screen coordinate,
 * the y axis is from top to bottom.
 */
void renderer2d_push_viewport(int x, int y, int w, int h);

void renderer2d_pop_viewport(void);

void renderer2d_get_viewport(int *o_x, int *o_y, int *o_w, int *o_h);


#endif /* RENDERER_H */
