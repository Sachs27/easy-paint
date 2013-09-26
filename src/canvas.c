#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <GL/glew.h>
#include <sf_utils.h>
#include <sf_array.h>
#include <sf_debug.h>

#include "app.h"
#include "canvas.h"
#include "load_shaders.h"


struct pixel {
    struct vec2 position;   /* normalized [-1, 1] */
    struct vec4 color;
};

struct canvas_tile {
    struct sf_rect      area;

    int                 isdirty;
    struct sf_rect      dirty_rect;

    uint8_t            *colors;     /* R-G-B-A (8-bit each) */

    struct texture     *texture;
};


#define CANVAS_TILE_INIT_NPIXELS 1024


static GLuint canvas_fbo;

static GLuint canvas_prog = 0;
static struct shader_info canvas_shaders[] = {
    {GL_VERTEX_SHADER, "canvas.vs.glsl"},
    {GL_FRAGMENT_SHADER, "canvas.fs.glsl"},
    {GL_NONE, NULL}
};

static struct vec2 vtexcoord[4] = {
    {0.0f, 0.0f},   /* left-top */
    {0.0f, 1.0f},   /* left-bottom */
    {1.0f, 1.0f},   /* right-bottom */
    {1.0f, 0.0f},   /* right-top */
};

static struct vec2 vposition[4];

static GLuint canvas_vao = 0, canvas_vbo = 0;


static void init_canvas(void) {
    canvas_prog = load_shaders(canvas_shaders);

    glGenVertexArrays(1, &canvas_vao);
    glBindVertexArray(canvas_vao);

    glGenBuffers(1, &canvas_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, canvas_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vposition) + sizeof(vtexcoord),
                 NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vposition), sizeof(vtexcoord),
                    vtexcoord);
    /* vposition */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    /* vtexcoord */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0,
                          (void *) sizeof(vposition));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glGenFramebuffers(1, &canvas_fbo);
}

static void canvas_tile_init(struct canvas_tile *ct, int x, int y) {
    GLfloat oclear_color[4];

    ct->texture = texture_create_2d(CANVAS_TILE_WIDTH, CANVAS_TILE_HEIGHT);
    /* clear the texture's content */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, canvas_fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         ct->texture->tid, 0);

    glGetFloatv(GL_COLOR_CLEAR_VALUE, oclear_color);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(oclear_color[0], oclear_color[1],
                 oclear_color[2], oclear_color[3]);

    ct->area.x = x;
    ct->area.y = y;
    ct->area.w = ct->texture->w;
    ct->area.h = ct->texture->h;
    ct->isdirty = 0;
    ct->colors = calloc(ct->texture->w * ct->texture->h, 4 * sizeof(uint8_t));
}

static void canvas_tile_plot(struct canvas_tile *ct, int x, int y,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint8_t *color;

    if (ct->isdirty == 0) {
        ct->isdirty = 1;
        ct->dirty_rect.x = 0;
        ct->dirty_rect.y = 0;
        ct->dirty_rect.w = 0;
        ct->dirty_rect.h = 0;
    }
    /* conver coordinate to canvas tile's */
    x -= ct->area.x;
    y -= ct->area.y;

    color = ct->colors + 4 * (y * ct->texture->h + x);
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;

    if (!sf_rect_iscontain(&ct->dirty_rect, x, y)) {
        if (ct->dirty_rect.w == 0) {
            ct->dirty_rect.x = x;
            ct->dirty_rect.y = y;
            ct->dirty_rect.w = 1;
            ct->dirty_rect.h = 1;
        } else {
            int x1 = ct->dirty_rect.x + ct->dirty_rect.w;
            int y1 = ct->dirty_rect.y + ct->dirty_rect.h;

            if (x < ct->dirty_rect.x) {
                ct->dirty_rect.w += ct->dirty_rect.x - x;
                ct->dirty_rect.x = x;
            } else if (x >= x1) {
                ct->dirty_rect.w += x - x1 + 1;
            }

            if (y < ct->dirty_rect.y) {
                ct->dirty_rect.h += ct->dirty_rect.y - y;
                ct->dirty_rect.y = y;
            } else if (y >= y1) {
                ct->dirty_rect.h += y - y1 + 1;
            }
        }
    }
}

static void canvas_update_tile(struct canvas *canvas, struct canvas_tile *ct) {
    uint32_t colors[ct->dirty_rect.w * ct->dirty_rect.h];
    uint32_t *row;
    uint32_t *src_row;
    int i;

    src_row = ((uint32_t *) (ct->colors))
                                + ct->dirty_rect.y * ct->texture->w
                                + ct->dirty_rect.x;
    row = colors;

    for (i = 0; i < ct->dirty_rect.h; ++i) {
        memcpy(row, src_row, ct->dirty_rect.w * sizeof(uint32_t));
        row += ct->dirty_rect.w;
        src_row += ct->texture->w;
    }
    /* use dirty rect */
    glBindTexture(GL_TEXTURE_2D, ct->texture->tid);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    ct->dirty_rect.x, ct->dirty_rect.y,
                    ct->dirty_rect.w, ct->dirty_rect.h,
                    GL_RGBA, GL_UNSIGNED_BYTE, colors);
    glBindTexture(GL_TEXTURE_2D, 0);
    ct->isdirty = 0;
}

/**
 * @param x
 * @param y point which tile must contain.
 */
static struct canvas_tile *canvas_add_tile(struct canvas *canvas,
                                           int x, int y) {
    int xtile = 0, ytile = 0, xstep, ystep;
    struct canvas_tile ct;

    xstep = x > 0 ? CANVAS_TILE_WIDTH : -CANVAS_TILE_WIDTH;
    ystep = y > 0 ? CANVAS_TILE_HEIGHT : -CANVAS_TILE_HEIGHT;

    while (!((x - xtile) >= 0 && (x - xtile) < CANVAS_TILE_WIDTH)) {
        xtile += xstep;
    }

    while (!((y - ytile) >= 0 && (y - ytile) < CANVAS_TILE_WIDTH)) {
        ytile += ystep;
    }

    canvas_tile_init(&ct, xtile, ytile);

    dprintf("canvas has %d tiles\n", canvas->tiles->nelts + 1);

    return sf_list_push(canvas->tiles, &ct);
}

struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h) {
    struct canvas *canvas;

    if (canvas_prog == 0) {
        init_canvas();
    }

    canvas = malloc(sizeof(*canvas));
    assert(canvas != NULL);
    canvas->background = background;
    canvas->viewport.x = x;
    canvas->viewport.y = y;
    canvas->viewport.w = w;
    canvas->viewport.h = h;
    canvas->offset.x = 0;
    canvas->offset.y = 0;
    canvas->tiles = sf_list_create(sizeof(struct canvas_tile));

    return canvas;
}

void canvas_draw(struct canvas *canvas) {
    struct sf_rect rect_camera;
    GLint oviewport[4];

    glGetIntegerv(GL_VIEWPORT, oviewport);
    glViewport(canvas->viewport.x,
               g_app.window->h - (canvas->viewport.y + canvas->viewport.h),
               canvas->viewport.w, canvas->viewport.h);

    /* draw background */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    /* draw pixels */
    rect_camera.x = canvas->offset.x;
    rect_camera.y = canvas->offset.y;
    rect_camera.w = canvas->viewport.w;
    rect_camera.h = canvas->viewport.h;

    glUseProgram(canvas_prog);
    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (!sf_rect_isintersect(&rect_camera, &ct->area)) {
            continue;
        }
        if (ct->isdirty) {
            canvas_update_tile(canvas, ct);
        }
        glBindVertexArray(canvas_vao);
        glBindBuffer(GL_ARRAY_BUFFER, canvas_vbo);
        vposition[0].x = ct->area.x - canvas->offset.x;
        vposition[0].y = canvas->viewport.h - (ct->area.y - canvas->offset.y);
        vposition[0].x = vposition[0].x * 2.0f / canvas->viewport.w - 1.0f;
        vposition[0].y = vposition[0].y * 2.0f / canvas->viewport.h - 1.0f;
        float nw = ((float) ct->texture->w) / canvas->viewport.w * 2.0f;
        float nh = ((float) ct->texture->h) / canvas->viewport.h * 2.0f;
        vposition[1].x = vposition[0].x;
        vposition[1].y = vposition[0].y - nh;
        vposition[2].x = vposition[0].x + nw;
        vposition[2].y = vposition[1].y;
        vposition[3].x = vposition[2].x;
        vposition[3].y = vposition[0].y;

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ct->texture->tid);
        glUniform1i(glGetUniformLocation(canvas_prog, "tex0"), 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    SF_LIST_END();
    glUseProgram(0);

    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
}

void canvas_plot(struct canvas *canvas, int x, int y,
                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    /* convert coordinate to canvas' */
    x += canvas->offset.x - canvas->viewport.x;
    y += canvas->offset.y - canvas->viewport.y;

    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            return canvas_tile_plot(ct, x, y, r, g, b, a);
        }
    SF_LIST_END();

    canvas_tile_plot(canvas_add_tile(canvas, x, y), x, y, r, g, b, a);
}

void canvas_pick(struct canvas *canvas, int x, int y,
                 uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
    /* convert coordinate to canvas' */
    x += canvas->offset.x - canvas->viewport.x;
    y += canvas->offset.y - canvas->viewport.y;

    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            uint8_t *color;
            /* conver coordinate to canvas tile's */
            x -= ct->area.x;
            y -= ct->area.y;

            color = ct->colors + 4 * (y * ct->texture->h + x);
            if (r) {
                *r = color[0];
            }
            if (g) {
                *g = color[1];
            }
            if (b) {
                *b = color[2];
            }
            if (a) {
                *a = color[3];
            }
            return;
        }
    SF_LIST_END();

    if (r) {
        *r = 0;
    }
    if (g) {
        *g = 0;
    }
    if (b) {
        *b = 0;
    }
    if (a) {
        *a = 0;
    }
}

void canvas_offset(struct canvas *canvas, int xoff, int yoff) {
    canvas->offset.x += xoff;
    canvas->offset.y += yoff;
}
