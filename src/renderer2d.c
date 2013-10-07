#include <stdlib.h>
#include <GL/glew.h>

#include "sf_rect.h"
#include "load_shaders.h"

#include "renderer2d.h"


static int                  isinited = 0;
static GLuint               renderer2d_vbo = 0;
static GLsizeiptr           renderer2d_vbo_size = 1024;
static GLuint               renderer2d_fbo = 0;
static GLuint               renderer2d_line_prog = 0;
static struct shader_info   renderer2d_line_shaders[] = {
    {GL_VERTEX_SHADER, "renderer2d_line.vs.glsl"},
    {GL_FRAGMENT_SHADER, "renderer2d_line.fs.glsl"},
    {GL_NONE, NULL}
};
static GLuint   renderer2d_texture_prog = 0;
static struct shader_info renderer2d_texture_shaders[] = {
    {GL_VERTEX_SHADER, "renderer2d_texture.vs.glsl"},
    {GL_FRAGMENT_SHADER, "renderer2d_texture.fs.glsl"},
    {GL_NONE, NULL}
};

static void renderer2d_init(void) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glGenBuffers(1, &renderer2d_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer2d_vbo);
    glBufferData(GL_ARRAY_BUFFER, renderer2d_vbo_size, NULL, GL_DYNAMIC_DRAW);

    glGenFramebuffers(1, &renderer2d_fbo);

    renderer2d_line_prog = load_shaders(renderer2d_line_shaders);
    renderer2d_texture_prog = load_shaders(renderer2d_texture_shaders);

    isinited = 1;
}

/**
 * Set the top one of the 'viewports' to the current viewport.
 */
static void renderer2d_set_viewport(struct renderer2d *r) {
    struct sf_rect *window = SF_ARRAY_HEAD(r->viewports);
    struct sf_rect *viewport = SF_ARRAY_TAIL(r->viewports);
    int x = viewport->x;
    int y = viewport->y;
    int w = viewport->w;
    int h = viewport->h;

    glViewport(x, window->h - y - h, w, h);
    glScissor(x, window->h - y - h, w, h);

    mat4_orthographic(&r->projection, 0, w, h, 0, 1.0f, -1.0f);
}


struct renderer2d *renderer2d_create(int w, int h) {
    struct renderer2d *r;

    if (!isinited) {
        renderer2d_init();
    }
    r = malloc(sizeof(*r));
    r->w = w;
    r->h = h;
    r->viewports = sf_array_create(sizeof(struct sf_rect), SF_ARRAY_NALLOC);
    renderer2d_push_viewport(r, 0, 0, w, h);
    renderer2d_set_render_target(r, NULL);

    return r;
}

void renderer2d_resize(struct renderer2d *r, int w, int h) {
    struct sf_rect *viewport = SF_ARRAY_HEAD(r->viewports);

    r->w = w;
    r->h = h;

    if (r->render_target == NULL) {
        viewport->w = w;
        viewport->h = h;
    }

    renderer2d_set_viewport(r);
}

void renderer2d_clear(struct renderer2d *renderer,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer2d_set_render_target(struct renderer2d *r, struct texture *rt) {
    struct sf_rect *viewport = SF_ARRAY_HEAD(r->viewports);
    r->render_target = rt;

    if (r->render_target) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer2d_fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, r->render_target->tid, 0);
        viewport->w = rt->w;
        viewport->h = rt->h;
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        viewport->w = r->w;
        viewport->h = r->h;
    }

    renderer2d_set_viewport(r);
}

void renderer2d_draw_line(struct renderer2d *renderer, float width,
                          int x0, int y0, int x1, int y1,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    struct vec2 vposition[2] = { {x0, y0}, {x1, y1} };

    renderer2d_set_viewport(renderer);

    glUseProgram(renderer2d_line_prog);

    glBindBuffer(GL_ARRAY_BUFFER, renderer2d_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(renderer2d_line_prog, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(renderer2d_line_prog,
                                            "mprojection"),
                       1, MATRIX_GL_TRANSPOSE,
                       (GLfloat *) &renderer->projection);

    glLineWidth(width);
    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

static void renderer2d_draw_texture_inner(struct renderer2d *r,
                                          int dx, int dy, int dw, int dh,
                                          struct texture *texture,
                                          int sx, int sy, int sw, int sh) {
    struct sf_rect *viewport;
    struct vec2 vposition[4];
    struct vec2 vtexcoord[4];
    float ntw, nth;

    viewport = SF_ARRAY_TAIL(r->viewports);
    if (dw == 0) {
        dw = viewport->w;
    }
    if (dh == 0) {
        dh = viewport->h;
    }
    vposition[0].x = dx;
    vposition[0].y = dy;
    vposition[1].x = dx;
    vposition[1].y = dy + dh;
    vposition[2].x = dx + dw;
    vposition[2].y = vposition[1].y;
    vposition[3].x = vposition[2].x;
    vposition[3].y = dy;

    if (sw == 0) {
        ntw = 1.0f;
    } else {
        ntw = ((float) sw) / texture->w;
    }
    if (sh == 0) {
        nth = 1.0f;
    } else {
        nth = ((float) sh) / texture->h;
    }
    vtexcoord[0].x = ((float) sx) / texture->w;
    vtexcoord[0].y = ((float) sy) / texture->h;
    vtexcoord[1].x = vtexcoord[0].x;
    vtexcoord[1].y = vtexcoord[0].y + nth;
    vtexcoord[2].x = vtexcoord[0].x + ntw;
    vtexcoord[2].y = vtexcoord[1].y;
    vtexcoord[3].x = vtexcoord[2].x;
    vtexcoord[3].y = vtexcoord[0].y;

    glBindBuffer(GL_ARRAY_BUFFER, renderer2d_vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vposition), sizeof(vtexcoord),
                    vtexcoord);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0,
                          (void *) sizeof(vposition));
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tid);
    glUniform1i(glGetUniformLocation(renderer2d_texture_prog, "texture0"), 0);

    glUniformMatrix4fv(glGetUniformLocation(renderer2d_texture_prog,
                                            "mprojection"),
                       1, MATRIX_GL_TRANSPOSE, (GLfloat *) &r->projection);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderer2d_draw_texture(struct renderer2d *r,
                             int dx, int dy, int dw, int dh,
                             struct texture *texture,
                             int sx, int sy, int sw, int sh) {
    glUseProgram(renderer2d_texture_prog);

    glUniform1i(glGetUniformLocation(renderer2d_texture_prog, "iscoloring"),
                                     0);

    renderer2d_draw_texture_inner(r, dx, dy, dw, dh, texture, sx, sy, sw, sh);

    glUseProgram(0);
}

void renderer2d_draw_texture_with_color(struct renderer2d *renderer,
                                        int dx, int dy, int dw, int dh,
                                        struct texture *texture,
                                        int sx, int sy, int sw, int sh,
                                        uint8_t r, uint8_t g, uint8_t b) {
    glUseProgram(renderer2d_texture_prog);

    glUniform1i(glGetUniformLocation(renderer2d_texture_prog, "iscoloring"),
                                     1);

    glUniform3f(glGetUniformLocation(renderer2d_texture_prog, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f);

    renderer2d_draw_texture_inner(renderer, dx, dy, dw, dh,
                                  texture, sx, sy, sw, sh);

    glUseProgram(0);
}


void renderer2d_push_viewport(struct renderer2d *r,
                              int x, int y, int w, int h) {
    struct sf_rect viewport;

    viewport.x = x;
    viewport.y = y;
    viewport.w = w;
    viewport.h = h;

    sf_array_push(r->viewports, &viewport);

    renderer2d_set_viewport(r);
}

void renderer2d_pop_viewport(struct renderer2d *r) {
    if (r->viewports->nelts > 1) {
        sf_array_pop(r->viewports, NULL);
    }

    renderer2d_set_viewport(r);
}
