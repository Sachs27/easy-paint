#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <sf_utils.h>

#include "app.h"
#include "canvas.h"
#include "load_shaders.h"


static GLuint canvas_prog = 0;
static struct shader_info canvas_shaders[] = {
    {GL_VERTEX_SHADER, "canvas.vs.glsl"},
    {GL_FRAGMENT_SHADER, "canvas.fs.glsl"},
    {GL_NONE, NULL}
};

static struct vertex {
    GLfloat x, y;
    GLfloat u, v;
} quad_data[4] = {
    {-1.0f, 1.0f, 0.0f, 0.0f},
    {-1.0f,-1.0f, 0.0f, 1.0f},
    { 1.0f,-1.0f, 1.0f, 1.0f},
    { 1.0f, 1.0f, 1.0f, 0.0f},
};


static void canvas_tile_set_color(struct canvas_tile *ct, int mode,
                                  int x, int y, scalar_t r, scalar_t g,
                                  scalar_t b, scalar_t a) {
    /* blend color */
    struct vec4 *color = ct->colors + (y * ct->area.w + x);

    color->r = color->r * (1.0f - a) + r * a;
    SCALAR_CLAMP(color->r, 0.0f, 1.0f);

    color->g = color->g * (1.0f - a) + g * a;
    SCALAR_CLAMP(color->g, 0.0f, 1.0f);

    color->b = color->b * (1.0f - a) + b * a;
    SCALAR_CLAMP(color->b, 0.0f, 1.0f);

    color->a = color->a + a;
    SCALAR_CLAMP(color->a, 0.0f, 1.0f);

    if (ct->isdirty) {
        /* calculate the dirty rectangle */
    } else {
        ct->isdirty = 1;
        ct->dirty_rect.x = x;
        ct->dirty_rect.y = y;
        ct->dirty_rect.w = 1;
        ct->dirty_rect.h = 1;
    }
}


struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h) {
    struct canvas *canvas;

    if (canvas_prog == 0) {
        canvas_prog = load_shaders(canvas_shaders);
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


    glGenVertexArrays(1, &canvas->vao);
    glBindVertexArray(canvas->vao);
    glGenBuffers(1, &canvas->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, canvas->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    /* vposition */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);
    glEnableVertexAttribArray(0);
    /* vtexcoord */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
                          &((struct vertex *) 0)->u);
    glEnableVertexAttribArray(1);

    struct canvas_tile ct;
    ct.texture = texture_create_2d(sf_uint_next_power_of_two(w),
                                   sf_uint_next_power_of_two(h));
    ct.colors = calloc(w * h, sizeof(*ct.colors));
    glBindTexture(GL_TEXTURE_2D, ct.texture->tid);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ct.texture->w, ct.texture->h,
                    GL_RGBA, GL_FLOAT, ct.colors);
    glBindTexture(GL_TEXTURE_2D, 0);
    ct.area.x = ct.area.y = 0;
    ct.area.w = ct.texture->w;
    ct.area.h = ct.texture->h;
    ct.isdirty = 0;
    ct.dirty_rect.x = ct.dirty_rect.y = ct.dirty_rect.w = ct.dirty_rect.h = 0;
    sf_list_push(canvas->tiles, &ct);

    return canvas;
}

void canvas_draw(struct canvas *canvas) {
    GLint oviewport[4];

    glGetIntegerv(GL_VIEWPORT, oviewport);
    glViewport(canvas->viewport.x,
               g_app.window->h - (canvas->viewport.y + canvas->viewport.h),
               canvas->viewport.w, canvas->viewport.h);
    glUseProgram(canvas_prog);
    glBindVertexArray(canvas->vao);


    /* draw background */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    /* draw pixels */
    /* 未完全实现，目前只画第一个 tile */
    struct canvas_tile *ct = canvas->tiles->head->next->elt;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ct->texture->tid);
    if (ct->isdirty) {
        /* 这里更新了全部 texture， 没用使用脏矩阵 */
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0, ct->texture->w, ct->texture->h,
                        GL_RGBA, GL_FLOAT, ct->colors);
        ct->isdirty = 0;
    }

    glUniform1i(glGetUniformLocation(canvas_prog, "tex0"), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glUseProgram(0);

    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
}

void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      scalar_t r, scalar_t g, scalar_t b, scalar_t a) {
    SF_LIST_BEGIN(canvas->tiles, struct canvas_tile, ct);
        if (sf_rect_iscontain(&ct->area, x, y)) {
            canvas_tile_set_color(ct, mode,
                                  x - ct->area.x, y - ct->area.y,
                                  r, g, b, a);
            return;
        }
    SF_LIST_END();

    /* create new canvas tile */
}
