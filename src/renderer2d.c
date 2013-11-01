#include <stdlib.h>

#ifdef GLES2
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#else
# include <GL/glew.h>
#endif

#include <sf_debug.h>

#include "sf_rect.h"

#include "renderer2d.h"


struct shader_info {
    GLenum       type;
    const char  *source;
    GLuint       shader;
};


static GLsizeiptr           renderer2d_vbo_size = 1024;

static struct shader_info   renderer2d_rect_shaders[] = {
    {GL_VERTEX_SHADER,
#ifndef GLES2
        "#version 130                                           \n"
#endif
        "uniform mat4 mprojection;                              \n"
        "attribute vec2 vposition;                              \n"
        "void main(void) {                                      \n"
        "    gl_Position = mprojection * vec4(vposition, 0, 1); \n"
        "}                                                      \n"
    },
    {GL_FRAGMENT_SHADER,
#ifndef GLES2
        "#version 130                                           \n"
#endif
        "uniform vec4 color;                                    \n"
        "void main(void) {                                      \n"
        "   gl_FragColor = color;                               \n"
        "}                                                      \n"
    },
    {GL_NONE, NULL}
};
static struct shader_info renderer2d_texture_shaders[] = {
    {GL_VERTEX_SHADER,
#ifndef GLES2
        "#version 130                                           \n"
#endif
        "uniform mat4 mprojection;                              \n"
        "attribute vec2 vposition;                              \n"
        "attribute vec2 vtexcoord;                              \n"
        "varying vec2 texcoord;                                 \n"
        "void main(void) {                                      \n"
        "    gl_Position = mprojection * vec4(vposition, 0, 1); \n"
        "    texcoord = vtexcoord;                              \n"
        "}                                                      \n"
    },
    {GL_FRAGMENT_SHADER,
#ifndef GLES2
        "#version 130                                       \n"
#else
        "precision lowp float;                           \n"
#endif
        "uniform sampler2D texture0;                        \n"
        "uniform bool      iscoloring;                      \n"
        "uniform vec3      color;                           \n"
        "varying vec2 texcoord;                             \n"
        "void main(void) {                                  \n"
        "    vec4 texcolor = texture2D(texture0, texcoord); \n"
        "    if (iscoloring) {                              \n"
        "        gl_FragColor = vec4(color.rgb * texcolor.a,\n"
        "                            texcolor.a);           \n"
        "    } else {                                       \n"
        "        gl_FragColor = texcolor;                   \n"
        "    }                                              \n"
        "}                                                  \n"
    },
    {GL_NONE, NULL}
};

static int attach_shader(GLuint program, struct shader_info *info) {
    GLint   compile_status;
    GLuint  shader;
    const GLchar *psrc = info->source;

    shader = glCreateShader(info->type);
    info->shader = shader;

    glShaderSource(info->shader, 1, &psrc, NULL);

    /* compile shader */
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE) {
#ifndef NDEBUG
        GLsizei len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetShaderInfoLog(shader, len, NULL, log);
            dprintf("%s\n", log);
        }
#endif /* NDEBUG */
        return -1;
    }

    glAttachShader(program, shader);
    return 0;
}

GLuint load_shaders(struct shader_info *info) {
    struct shader_info *entry;
    GLint               link_status;
    GLuint              program;

    program = glCreateProgram();

    for (entry = info; entry->type != GL_NONE; ++entry) {
        entry->shader = 0;
        if (attach_shader(program, entry) != 0) {
            goto load_shaders_fail;
        }
    }

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
#ifndef NDEBUG
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetProgramInfoLog(program, len, NULL, log);
            dprintf("Failed to link program: %s\n", log);
        }
#endif /* NDEBUG */
        goto load_shaders_fail;
    }

    return program;

load_shaders_fail:
    for (entry = info; entry->type != GL_NONE; ++entry) {
        if (entry->shader) {
            glDeleteShader(entry->shader);
            entry->shader = 0;
        }
    }
    glDeleteProgram(program);
    return 0;
}
#if 0
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
#endif

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

    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    r = malloc(sizeof(*r));
    r->w = w;
    r->h = h;
    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, renderer2d_vbo_size, NULL, GL_DYNAMIC_DRAW);
    glGenFramebuffers(1, &r->fbo);
    r->prog_rect = load_shaders(renderer2d_rect_shaders);
    r->prog_texture = load_shaders(renderer2d_texture_shaders);
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
        glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, r->render_target->tid, 0);
        viewport->w = rt->w;
        viewport->h = rt->h;
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        viewport->w = r->w;
        viewport->h = r->h;
    }

    renderer2d_set_viewport(r);
}

void renderer2d_fill_rect(struct renderer2d *renderer, int x, int y,
                          int w, int h,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    struct vec2 vposition[4] = {
        {x, y},
        {x, y + h},
        {x + w, y + h},
        {x + w, y}
    };

    glUseProgram(renderer->prog_rect);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);

    glVertexAttribPointer(glGetAttribLocation(renderer->prog_rect, "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(renderer->prog_rect, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(renderer->prog_rect, "mprojection"),
                       1, MATRIX_GL_TRANSPOSE,
                       (GLfloat *) &renderer->projection);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void renderer2d_draw_line(struct renderer2d *renderer, float width,
                          int x0, int y0, int x1, int y1,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    struct vec2 vposition[2] = { {x0, y0}, {x1, y1} };

    glUseProgram(renderer->prog_rect);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(glGetAttribLocation(renderer->prog_rect, "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(renderer->prog_rect, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(renderer->prog_rect,
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

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(glGetAttribLocation(r->prog_texture,
                                              "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vposition), sizeof(vtexcoord),
                    vtexcoord);
    glVertexAttribPointer(glGetAttribLocation(r->prog_texture, "vtexcoord"),
                          2, GL_FLOAT, GL_FALSE, 0, (void *)sizeof(vposition));
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tid);
    glUniform1i(glGetUniformLocation(r->prog_texture, "texture0"), 0);

    glUniformMatrix4fv(glGetUniformLocation(r->prog_texture, "mprojection"),
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
    glUseProgram(r->prog_texture);

    glUniform1i(glGetUniformLocation(r->prog_texture, "iscoloring"), 0);

    renderer2d_draw_texture_inner(r, dx, dy, dw, dh, texture, sx, sy, sw, sh);

    glUseProgram(0);
}

void renderer2d_draw_texture_with_color(struct renderer2d *renderer,
                                        int dx, int dy, int dw, int dh,
                                        struct texture *texture,
                                        int sx, int sy, int sw, int sh,
                                        uint8_t r, uint8_t g, uint8_t b) {
    glUseProgram(renderer->prog_texture);

    glUniform1i(glGetUniformLocation(renderer->prog_texture, "iscoloring"),
                                     1);

    glUniform3f(glGetUniformLocation(renderer->prog_texture, "color"),
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

void renderer2d_get_viewport(struct renderer2d *r, int *o_x, int *o_y,
                             int *o_w, int *o_h) {
    struct sf_rect *vp = SF_ARRAY_TAIL(r->viewports);
#define PTR_ASSIGN(ptr, val) do { if (ptr) { (*ptr) = val; } } while (0)
    PTR_ASSIGN(o_x, vp->x);
    PTR_ASSIGN(o_y, vp->y);
    PTR_ASSIGN(o_w, vp->w);
    PTR_ASSIGN(o_h, vp->h);
}
