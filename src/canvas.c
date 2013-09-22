#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <sf_utils.h>
#include <sf_array.h>

#include "app.h"
#include "canvas.h"
#include "load_shaders.h"


struct pixel {
    struct vec2 position;   /* normalized [-1, 1] */
    struct vec4 color;
};

struct canvas_tile {
    struct sf_rect area;

    int isdirty;
    struct sf_array *dirty_pixels;

    struct texture *texture;
};


static GLuint canvas_fbo;
static GLuint canvas_texture_prog = 0;
static struct shader_info canvas_texture_shaders[] = {
    {GL_VERTEX_SHADER, "canvas_texture.vs.glsl"},
    {GL_FRAGMENT_SHADER, "canvas_texture.fs.glsl"},
    {GL_NONE, NULL}
};
static GLuint canvas_texture_vao = 0, canvas_texture_vbo = 0;
static int canvas_texture_vbo_size = 0;
#define CANVAS_TEXTURE_INIT_SIZE 1024

static GLuint canvas_prog = 0;
static struct shader_info canvas_shaders[] = {
    {GL_VERTEX_SHADER, "canvas.vs.glsl"},
    {GL_FRAGMENT_SHADER, "canvas.fs.glsl"},
    {GL_NONE, NULL}
};

static struct vec2 vtexcoord[4] = {
    {0.0f, 1.0f},   /* left-top */
    {0.0f, 0.0f},   /* left-bottom */
    {1.0f, 0.0f},   /* right-bottom */
    {1.0f, 1.0f},   /* right-top */
};

static struct vec2 vposition[4];

static GLuint canvas_vao = 0, canvas_vbo = 0;


/**
 * @param x
 * @param y point relative to the origin of tile's texture.
 */
static void canvas_tile_set_color(struct canvas_tile *ct, int mode,
                                  int x, int y, scalar_t r, scalar_t g,
                                  scalar_t b, scalar_t a) {
    struct pixel p;

    /* let the origin at the left-bottom corner */
    y = ct->texture->h - y;

    ct->isdirty = 1;
    p.color.r = r;
    p.color.g = g;
    p.color.b = b;
    p.color.a = a;
    p.position.x = x * 2.0f / ct->texture->w - 1.0f;
    p.position.y = y * 2.0f / ct->texture->h - 1.0f;

    sf_array_push(ct->dirty_pixels, &p);
}

static void canvas_tile_update(struct canvas_tile *ct) {
    GLint oviewport[4];
    GLint oprog;

    if (canvas_fbo == 0) {
        glGenFramebuffers(1, &canvas_fbo);
    }

    if (canvas_texture_vao == 0 || canvas_texture_vbo == 0) {
        glGenVertexArrays(1, &canvas_texture_vao);
        glBindVertexArray(canvas_texture_vao);

        glGenBuffers(1, &canvas_texture_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, canvas_texture_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     CANVAS_TEXTURE_INIT_SIZE * sizeof(struct pixel),
                     NULL, GL_DYNAMIC_DRAW);
        canvas_texture_vbo_size = CANVAS_TEXTURE_INIT_SIZE;

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct pixel),
                              0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct pixel),
                              &((struct pixel *) 0)->color);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

    /* save state */
    glGetIntegerv(GL_CURRENT_PROGRAM, &oprog);
    glGetIntegerv(GL_VIEWPORT, oviewport);

    /* update texture */
    glUseProgram(canvas_texture_prog);
    glBindVertexArray(canvas_texture_vao);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, canvas_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, ct->texture->tid, 0);
    glViewport(0, 0, ct->texture->w, ct->texture->h);

    glBindBuffer(GL_ARRAY_BUFFER, canvas_texture_vbo);
    if (canvas_texture_vbo_size < ct->dirty_pixels->nalloc) {
        glBufferData(GL_ARRAY_BUFFER,
                     ct->dirty_pixels->nalloc * sizeof(struct pixel),
                     NULL, GL_DYNAMIC_DRAW);
        canvas_texture_vbo_size = ct->dirty_pixels->nalloc;
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    ct->dirty_pixels->nelts * sizeof(struct pixel),
                    SF_ARRAY_NTH(ct->dirty_pixels, 0));

    glDrawArrays(GL_POINTS, 0, ct->dirty_pixels->nelts);

    /* restore previous state */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(oprog);
    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);

    ct->isdirty = 0;
    sf_array_clear(ct->dirty_pixels, NULL);
}

struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h) {
    struct canvas *canvas;
    struct canvas_tile ct;

    if (canvas_prog == 0) {
        canvas_prog = load_shaders(canvas_shaders);
    }

    if (canvas_texture_prog == 0) {
        canvas_texture_prog = load_shaders(canvas_texture_shaders);
    }

    if (canvas_vao == 0 || canvas_vbo == 0) {
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

    ct.texture = texture_create_2d(sf_uint_next_power_of_two(w),
                                   sf_uint_next_power_of_two(h));
    /* need to clear the texture's content */

    ct.area.x = ct.area.y = 0;
    ct.area.w = ct.texture->w;
    ct.area.h = ct.texture->h;
    ct.isdirty = 0;
    ct.dirty_pixels = sf_array_create(sizeof(struct pixel),
                                      CANVAS_TEXTURE_INIT_SIZE);
    sf_list_push(canvas->tiles, &ct);

    return canvas;
}

void canvas_draw(struct canvas *canvas) {
    GLint oviewport[4];
    struct canvas_tile *ct = canvas->tiles->head->next->elt;


    glGetIntegerv(GL_VIEWPORT, oviewport);
    glViewport(canvas->viewport.x,
               g_app.window->h - (canvas->viewport.y + canvas->viewport.h),
               canvas->viewport.w, canvas->viewport.h);

    /* draw background */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    /* draw pixels */
    /* 未完全实现，目前只画第一个 tile */
    if (ct->isdirty) {
        canvas_tile_update(ct);
    }

    glUseProgram(canvas_prog);
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

    glUseProgram(0);

    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
}

void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      scalar_t r, scalar_t g, scalar_t b, scalar_t a) {
    x += canvas->offset.x - canvas->viewport.x;
    y += canvas->offset.y - canvas->viewport.y;


    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            canvas_tile_set_color(ct, mode, x - ct->area.x, y - ct->area.y,
                                  r, g, b, a);
            return;
        }
    SF_LIST_END();

    /* create new canvas tile */
}

void canvas_offset(struct canvas *canvas, int xoff, int yoff) {
    canvas->offset.x += xoff;
    canvas->offset.y += yoff;
}
