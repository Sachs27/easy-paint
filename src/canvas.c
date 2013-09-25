#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <sf_utils.h>
#include <sf_array.h>
#include <sf_debug.h>

#include "app.h"
#include "canvas.h"
#include "load_shaders.h"


#define CANVAS_INIT_NPIXELS 1024
#define CANVAS_TEXTURE_WIDTH 1024
#define CANVAS_TEXTURE_HEIGHT 512


static struct texture *canvas_texture_buffer;
static GLuint canvas_fbo;
static GLuint canvas_texture_vao = 0, canvas_texture_vbo = 0;
static int canvas_texture_vbo_size = 0;

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

    glGenVertexArrays(1, &canvas_texture_vao);
    glBindVertexArray(canvas_texture_vao);

    glGenBuffers(1, &canvas_texture_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, canvas_texture_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 CANVAS_INIT_NPIXELS * sizeof(struct vec2),
                 NULL, GL_DYNAMIC_DRAW);
    canvas_texture_vbo_size = CANVAS_INIT_NPIXELS;
    /* vposition */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);

    canvas_texture_buffer = texture_create_2d(CANVAS_TEXTURE_WIDTH,
                                              CANVAS_TEXTURE_HEIGHT);
}

static void canvas_update_texture(struct canvas *canvas) {
    GLuint prog = canvas->cur_brush->prog;
    GLint oviewport[4];
    GLint oprog;
    GLboolean oblend;

    /* save state */
    glGetIntegerv(GL_CURRENT_PROGRAM, &oprog);
    glGetIntegerv(GL_VIEWPORT, oviewport);
    glGetBooleanv(GL_BLEND, &oblend);


    /* copy current texture data to canvas_texture_buffer */
    glUseProgram(canvas_prog);
    glBindVertexArray(canvas_vao);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, canvas_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, canvas_texture_buffer->tid, 0);
    glViewport(0, 0, canvas->texture->w, canvas->texture->h);
    glBindBuffer(GL_ARRAY_BUFFER, canvas_vbo);
    vposition[0].x = -1.0f;
    vposition[0].y =  1.0f;
    vposition[1].x = vposition[0].x;
    vposition[1].y = vposition[0].y - 2.0f;
    vposition[2].x = vposition[0].x + 2.0f;
    vposition[2].y = vposition[1].y;
    vposition[3].x = vposition[2].x;
    vposition[3].y = vposition[0].y;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas->texture->tid);
    glUniform1i(glGetUniformLocation(canvas_prog, "tex0"), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glUseProgram(0);

    /* update texture */
    glUseProgram(prog);
    glBindVertexArray(canvas_texture_vao);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, canvas_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, canvas->texture->tid, 0);
    glViewport(0, 0, canvas->texture->w, canvas->texture->h);

    glBindBuffer(GL_ARRAY_BUFFER, canvas_texture_vbo);
    if (canvas_texture_vbo_size < canvas->dirty_pixels->nalloc) {
        glBufferData(GL_ARRAY_BUFFER,
                     canvas->dirty_pixels->nalloc * sizeof(struct vec2),
                     NULL, GL_DYNAMIC_DRAW);
        canvas_texture_vbo_size = canvas->dirty_pixels->nalloc;
    }

    glUniform1i(glGetUniformLocation(prog, "type"),
                rand() % canvas->cur_brush->maxtype);

    glUniform1f(glGetUniformLocation(prog, "point_size"),
                canvas->cur_brush->point_size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas_texture_buffer->tid);
    glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

    glUniform2f(glGetUniformLocation(prog, "window_size"),
                canvas->texture->w, canvas->texture->h);

    glUniform4fv(glGetUniformLocation(prog, "brush_color"),
                 1, (float *) &canvas->cur_brush->color);

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    canvas->dirty_pixels->nelts * sizeof(struct vec2),
                    SF_ARRAY_NTH(canvas->dirty_pixels, 0));

    glDisable(GL_BLEND);
    glDrawArrays(GL_POINTS, 0, canvas->dirty_pixels->nelts);

    /* restore previous state */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(oprog);
    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
    if (oblend == GL_TRUE) {
        glEnable(GL_BLEND);
    }

    canvas->isdirty = 0;
    sf_array_clear(canvas->dirty_pixels, NULL);
}

struct canvas *canvas_create(struct texture *background,
                             int x, int y, int w, int h) {
    struct canvas *canvas;
    GLfloat oclear_color[4];

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
    canvas->texture = texture_create_2d(CANVAS_TEXTURE_WIDTH,
                                        CANVAS_TEXTURE_HEIGHT);
    /* clear the texture's content */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, canvas_fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         canvas->texture->tid, 0);

    glGetFloatv(GL_COLOR_CLEAR_VALUE, oclear_color);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(oclear_color[0], oclear_color[1],
                 oclear_color[2], oclear_color[3]);

    canvas->isdirty = 0;
    canvas->dirty_pixels = sf_array_create(sizeof(struct vec2),
                                           CANVAS_INIT_NPIXELS);
    canvas->cur_brush = NULL;

    return canvas;
}

void canvas_draw(struct canvas *canvas) {
    GLint oviewport[4];

    glGetIntegerv(GL_VIEWPORT, oviewport);
    glViewport(canvas->viewport.x,
               g_app.window->h - (canvas->viewport.y + canvas->viewport.h),
               canvas->viewport.w, canvas->viewport.h);

    /* draw background */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    /* draw pixels */
    if (canvas->isdirty) {
        canvas_update_texture(canvas);
    }

    glUseProgram(canvas_prog);
    glBindVertexArray(canvas_vao);
    glBindBuffer(GL_ARRAY_BUFFER, canvas_vbo);
    vposition[0].x = 0 - canvas->offset.x;
    vposition[0].y = canvas->viewport.h - (0 - canvas->offset.y);

    vposition[0].x = vposition[0].x * 2.0f / canvas->viewport.w - 1.0f;
    vposition[0].y = vposition[0].y * 2.0f / canvas->viewport.h - 1.0f;
    float nw = ((float) canvas->texture->w) / canvas->viewport.w * 2.0f;
    float nh = ((float) canvas->texture->h) / canvas->viewport.h * 2.0f;
    vposition[1].x = vposition[0].x;
    vposition[1].y = vposition[0].y - nh;
    vposition[2].x = vposition[0].x + nw;
    vposition[2].y = vposition[1].y;
    vposition[3].x = vposition[2].x;
    vposition[3].y = vposition[0].y;

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas->texture->tid);
    glUniform1i(glGetUniformLocation(canvas_prog, "tex0"), 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glUseProgram(0);

    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
}

void canvas_plot(struct canvas *canvas, int x, int y) {
    /* convert coordinate to canvas' */
    x += canvas->offset.x - canvas->viewport.x;
    y += canvas->offset.y - canvas->viewport.y;

    if (x < canvas->texture->w && y < canvas->texture->h) {
        struct vec2 position;

        canvas->isdirty = 1;
        /* conver coordinate to canvas tile's */
        y = canvas->texture->h - y;

        position.x = x * 2.0f / canvas->texture->w - 1.0f;
        position.y = y * 2.0f / canvas->texture->h - 1.0f;

        sf_array_push(canvas->dirty_pixels, &position);
    }
}

void canvas_offset(struct canvas *canvas, int xoff, int yoff) {
    canvas->offset.x += xoff;
    canvas->offset.y += yoff;
}

void canvas_set_current_brush(struct canvas *canvas, struct brush *brush) {
    /* update the dirty tiles */
    if (canvas->cur_brush && canvas->cur_brush != brush && canvas->isdirty) {
        canvas_update_texture(canvas);
    }
    canvas->cur_brush = brush;
}
