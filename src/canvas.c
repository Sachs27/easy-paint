#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "app.h"
#include "canvas.h"
#include "load_shaders.h"


static GLuint canvas_prog = 0;
static struct shader_info canvas_shaders[] = {
    {GL_VERTEX_SHADER, "canvas.vs.glsl"},
    {GL_FRAGMENT_SHADER, "canvas.fs.glsl"},
    {GL_NONE, NULL}
};
static struct mat4 mmvp;


static struct pixel *find_pixel(struct canvas *canvas, int x, int y) {
    SF_ARRAY_BEGIN(canvas->pixels, struct pixel, ptr);
        if (ptr->position.x == x && ptr->position.y == y) {
            return ptr;
        }
    SF_ARRAY_END();

    return NULL;
}

/*
 * 现在是每次更新都全部替换 OpenGL 的 Buffer, 可以只部分替换。
 */
static void update_render_buf(struct canvas *canvas) {
    glBindVertexArray(canvas->vao);

    glBindBuffer(GL_ARRAY_BUFFER, canvas->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 canvas->pixels->nalloc * canvas->pixels->size,
                 SF_ARRAY_NTH(canvas->pixels, 0), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, canvas->pixels->size, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                         canvas->pixels->size, &((struct pixel *) 0)->color);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    canvas->isdirty = 0;
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
    canvas->pixels = sf_array_create(sizeof(struct pixel), 1024);
    glGenVertexArrays(1, &canvas->vao);
    glGenBuffers(1, &canvas->vbo);
    canvas->isdirty = 0;

    return canvas;
}

void canvas_draw(struct canvas *canvas) {
    struct mat4 tmp;
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
        update_render_buf(canvas);
    }

    glUseProgram(canvas_prog);
    glBindVertexArray(canvas->vao);

    mat4_orthographic(&mmvp, 0.0f, canvas->viewport.w, canvas->viewport.h,
                      0.0f, 1.0f, -1.0f);
    mat4_translate(&tmp, -canvas->offset.x,-canvas->offset.y, 0.0f);
    mat4_mul(&mmvp, &mmvp, &tmp);

    glUniformMatrix4fv(glGetUniformLocation(canvas_prog, "mmvp"), 1,
                       MATRIX_GL_TRANSPOSE, (float *) &mmvp);

    glDrawArrays(GL_POINTS, 0, canvas->pixels->nelts);
    glBindVertexArray(0);
    glUseProgram(0);

    glViewport(oviewport[0], oviewport[1], oviewport[2], oviewport[3]);
}

void canvas_set_pixel(struct canvas *canvas, int mode, int x, int y,
                      float r, float g, float b, float a) {
    struct pixel buf;
    struct pixel *ptr;

    if ((ptr = find_pixel(canvas, x, y)) == NULL) {
        memset(&buf, 0, sizeof(buf));
        buf.position.x = x;
        buf.position.y = y;
        sf_array_push(canvas->pixels, &buf);
        ptr = SF_ARRAY_NTH(canvas->pixels, canvas->pixels->nelts - 1);
    }

    ptr->color.r = ptr->color.r * (1.0f - a) + r * a;
    SCALAR_CLAMP(ptr->color.r, 0.0f, 1.0f);
    ptr->color.g += ptr->color.g * (1.0f - a) + g * a;
    SCALAR_CLAMP(ptr->color.g, 0.0f, 1.0f);
    ptr->color.b += ptr->color.b * (1.0f - a) + b * a;
    SCALAR_CLAMP(ptr->color.b, 0.0f, 1.0f);
    ptr->color.a = ptr->color.a * (1.0f - a) + a * a;
    SCALAR_CLAMP(ptr->color.a, 0.0f, 1.0f);

    canvas->isdirty = 1;
}
