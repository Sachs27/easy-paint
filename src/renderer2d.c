#ifdef GLES2
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#else
# include <GL/glew.h>
#endif

#include <sf/utils.h>
#include <sf/log.h>

#include "sf_rect.h"

#include "renderer2d.h"


static struct renderer2d {
    int                 w;
    int                 h;

    struct mat4         projection;

    sf_array_t          viewports;      /* elt: (struct sf_rect)
                                         * The first elt is always the
                                         * screen's area.  */
    struct texture     *render_target;

    GLuint              vbo, fbo;
    GLuint              prog_rect, prog_texture, prog_point;
} renderer2d;


struct shader_info {
    GLenum       type;
    const char  *source;
    GLuint       shader;
};


static GLsizeiptr     renderer2d_vbo_size = 4096;

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

static struct shader_info renderer2d_point_shaders[] = {
    {GL_VERTEX_SHADER,
#ifndef GLES2
"#version 130                                                       \n"
#endif
"uniform mat4 mprojection;                                          \n"
"uniform float upoint_size;                                         \n"
"attribute vec2 vposition;                                          \n"
"void main(void) {                                                  \n"
"    gl_Position = mprojection * vec4(vposition, 0, 1);             \n"
"    gl_PointSize = upoint_size;                                    \n"
"}"
    },
    {GL_FRAGMENT_SHADER,
#ifndef GLES2
"#version 130                                   \n"
#endif
"uniform sampler2D utarget;                     \n"
"uniform vec2 utexsize;                         \n"
"uniform vec4 ucolor;                           \n"
"void main(void) {                              \n"
"    vec2 t = gl_PointCoord - vec2(0.5);        \n"
"    vec2 pixelcoord;                           \n"
"    vec4 dst, o;                               \n"
"    float f = dot(t, t);                       \n"
"    if (f > 0.25) {                            \n"
"        discard;                               \n"
"    }                                          \n"
"    dst = texture2D(utarget, gl_FragCoord.xy / utexsize);                  \n"
"    o.a = dst.a * (1.0 - ucolor.a) + ucolor.a;                             \n"
"    o.rgb = (dst.rgb * dst.a * (1.0 - ucolor.a) + ucolor.rgb * ucolor.a)   \n"
"            / o.a;                                                         \n"
"    gl_FragColor = o;\n"
"}"
    },
    {GL_NONE, NULL}
};


static int attach_shader(GLuint program, struct shader_info *info)
{
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
        GLsizei len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetShaderInfoLog(shader, len, NULL, log);
            sf_log(SF_LOG_ERR, "%s\n", log);
        }
        return -1;
    }

    glAttachShader(program, shader);

    return 0;
}

static GLuint load_shaders(struct shader_info *info)
{
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
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetProgramInfoLog(program, len, NULL, log);
            sf_log(SF_LOG_ERR, "Failed to link program: %s\n", log);
        }
        goto load_shaders_fail;
    }

    for (entry = info; entry->type != GL_NONE; ++entry) {
        glDeleteShader(entry->shader);
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


/**
 * Set the top one of the 'viewports' to the current viewport.
 */
static void renderer2d_set_viewport(void)
{
    struct sf_rect *window = sf_array_head(&renderer2d.viewports);
    struct sf_rect *viewport = sf_array_tail(&renderer2d.viewports);
    int x = viewport->x;
    int y = viewport->y;
    int w = viewport->w;
    int h = viewport->h;

    glViewport(x, window->h - y - h, w, h);
    glScissor(x, window->h - y - h, w, h);

    mat4_orthographic(&renderer2d.projection, 0, w, h, 0, 1.0f, -1.0f);
}


int renderer2d_init(int w, int h)
{
    sf_array_def_t def;

    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
#ifndef GLES2
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif

    renderer2d.w = w;
    renderer2d.h = h;
    glGenBuffers(1, &renderer2d.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer2d.vbo);
    glBufferData(GL_ARRAY_BUFFER, renderer2d_vbo_size, NULL,
                 GL_DYNAMIC_DRAW);
    glGenFramebuffers(1, &renderer2d.fbo);
    renderer2d.prog_rect = load_shaders(renderer2d_rect_shaders);
    renderer2d.prog_texture = load_shaders(renderer2d_texture_shaders);
    renderer2d.prog_point = load_shaders(renderer2d_point_shaders);

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct sf_rect);
    sf_array_init(&renderer2d.viewports, &def);

    renderer2d_push_viewport(0, 0, w, h);
    renderer2d_set_render_target(NULL);
    return 0;
}

void renderer2d_destroy(void)
{
    glDeleteBuffers(1, &renderer2d.vbo);
    glDeleteFramebuffers(1, &renderer2d.fbo);
    glDeleteProgram(renderer2d.prog_rect);
    glDeleteProgram(renderer2d.prog_texture);

    sf_array_destroy(&renderer2d.viewports);
}

void renderer2d_resize(int w, int h)
{
    struct sf_rect *viewport = sf_array_head(&renderer2d.viewports);

    renderer2d.w = w;
    renderer2d.h = h;

    if (renderer2d.render_target == NULL) {
        viewport->w = w;
        viewport->h = h;
    }

    renderer2d_set_viewport();
}

void renderer2d_clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer2d_blend_points(struct texture *dst,
                             struct vec2 *points, size_t npoints, float size,
                             float r, float g, float b, float a)
{
    size_t bufsiz = npoints * sizeof(*points);
    GLuint loc_proj, loc_pos, loc_pointsize, loc_color, loc_target,
           loc_texsize;

    glDisable(GL_BLEND);

    glUseProgram(renderer2d.prog_point);

    loc_proj = glGetUniformLocation(renderer2d.prog_point, "mprojection");
    loc_pos = glGetAttribLocation(renderer2d.prog_point, "vposition");
    loc_pointsize = glGetUniformLocation(renderer2d.prog_point, "upoint_size");
    loc_color = glGetUniformLocation(renderer2d.prog_point, "ucolor");
    loc_target = glGetUniformLocation(renderer2d.prog_point, "utarget");
    loc_texsize = glGetUniformLocation(renderer2d.prog_point, "utexsize");

    glUniform1f(loc_pointsize, size);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dst->tid);
    glUniform1i(loc_target, 0);
    glUniform2f(loc_texsize, dst->w, dst->h);
    glUniformMatrix4fv(loc_proj, 1, MATRIX_GL_TRANSPOSE,
                       (GLfloat *) &renderer2d.projection);


    glBindBuffer(GL_ARRAY_BUFFER, renderer2d.vbo);
    if (renderer2d_vbo_size < bufsiz) {
        while (renderer2d_vbo_size < bufsiz) {
            renderer2d_vbo_size <<= 1;
        }
        glBufferData(GL_ARRAY_BUFFER, renderer2d_vbo_size, NULL,
                     GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, npoints * sizeof(*points), points);
    glVertexAttribPointer(loc_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(loc_pos);
#if 0
#ifndef GLES2
    glUniform4f(loc_color, r, g, b, a);
    {
        size_t ndrawed = 0;
        while (ndrawed < npoints) {
            glDrawArrays(GL_POINTS, ndrawed++, 1);
            glFlush();
        }
    }
#else
    glUniform4f(loc_color, r, g, b, 1.0f);
    glDrawArrays(GL_POINTS, 0, npoints);
#endif
#endif
    glUniform4f(loc_color, r, g, b, a);
    glDrawArrays(GL_POINTS, 0, npoints);

    glDisableVertexAttribArray(loc_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    glEnable(GL_BLEND);
}

void renderer2d_set_render_target(struct texture *rt)
{
    struct sf_rect *viewport = sf_array_head(&renderer2d.viewports);
    renderer2d.render_target = rt;

    if (renderer2d.render_target) {
        glBindFramebuffer(GL_FRAMEBUFFER, renderer2d.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, renderer2d.render_target->tid, 0);
        viewport->w = rt->w;
        viewport->h = rt->h;
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        viewport->w = renderer2d.w;
        viewport->h = renderer2d.h;
    }

    renderer2d_set_viewport();
}

void renderer2d_fill_rect(int x, int y, int w, int h,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    struct vec2 vposition[4] = {
        {x, y},
        {x, y + h},
        {x + w, y + h},
        {x + w, y}
    };

    glUseProgram(renderer2d.prog_rect);

    glBindBuffer(GL_ARRAY_BUFFER, renderer2d.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);

    glVertexAttribPointer(glGetAttribLocation(renderer2d.prog_rect,
                                              "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(renderer2d.prog_rect, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(renderer2d.prog_rect,
                                            "mprojection"),
                       1, MATRIX_GL_TRANSPOSE,
                       (GLfloat *) &renderer2d.projection);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void renderer2d_draw_line(float width, int x0, int y0, int x1, int y1,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    struct vec2 vposition[2] = { {x0, y0}, {x1, y1} };

    glUseProgram(renderer2d.prog_rect);

    glBindBuffer(GL_ARRAY_BUFFER, renderer2d.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(glGetAttribLocation(renderer2d.prog_rect,
                                              "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(renderer2d.prog_rect, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(renderer2d.prog_rect,
                                            "mprojection"),
                       1, MATRIX_GL_TRANSPOSE,
                       (GLfloat *) &renderer2d.projection);

    glLineWidth(width);
    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

static void renderer2d_draw_texture_inner(int dx, int dy, int dw, int dh,
                                          struct texture *texture,
                                          int sx, int sy, int sw, int sh)
{
    struct sf_rect *viewport;
    struct vec2 vposition[4];
    struct vec2 vtexcoord[4];
    float ntw, nth;

    viewport = sf_array_tail(&renderer2d.viewports);
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

    glBindBuffer(GL_ARRAY_BUFFER, renderer2d.vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vposition), vposition);
    glVertexAttribPointer(glGetAttribLocation(renderer2d.prog_texture,
                                              "vposition"),
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vposition), sizeof(vtexcoord),
                    vtexcoord);
    glVertexAttribPointer(glGetAttribLocation(renderer2d.prog_texture, "vtexcoord"),
                          2, GL_FLOAT, GL_FALSE, 0, (void *)sizeof(vposition));
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tid);
    glUniform1i(glGetUniformLocation(renderer2d.prog_texture, "texture0"), 0);

    glUniformMatrix4fv(glGetUniformLocation(renderer2d.prog_texture, "mprojection"),
                       1, MATRIX_GL_TRANSPOSE, (GLfloat *) &renderer2d.projection);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderer2d_draw_texture(int dx, int dy, int dw, int dh,
                             struct texture *texture,
                             int sx, int sy, int sw, int sh)
{
    glUseProgram(renderer2d.prog_texture);

    glUniform1i(glGetUniformLocation(renderer2d.prog_texture, "iscoloring"), 0);

    renderer2d_draw_texture_inner(dx, dy, dw, dh, texture, sx, sy, sw, sh);

    glUseProgram(0);
}

void renderer2d_draw_texture_with_color(int dx, int dy, int dw, int dh,
                                        struct texture *texture,
                                        int sx, int sy, int sw, int sh,
                                        uint8_t r, uint8_t g, uint8_t b)
{
    glUseProgram(renderer2d.prog_texture);

    glUniform1i(glGetUniformLocation(renderer2d.prog_texture, "iscoloring"),
                                     1);

    glUniform3f(glGetUniformLocation(renderer2d.prog_texture, "color"),
                r / 255.0f, g / 255.0f, b / 255.0f);

    renderer2d_draw_texture_inner(dx, dy, dw, dh,
                                  texture, sx, sy, sw, sh);

    glUseProgram(0);
}


void renderer2d_push_viewport(int x, int y, int w, int h)
{
    struct sf_rect viewport;

    viewport.x = x;
    viewport.y = y;
    viewport.w = w;
    viewport.h = h;

    sf_array_push(&renderer2d.viewports, &viewport);

    renderer2d_set_viewport();
}

void renderer2d_pop_viewport(void)
{
    if (sf_array_cnt(&renderer2d.viewports) > 1) {
        sf_array_pop(&renderer2d.viewports);
    }

    renderer2d_set_viewport();
}

void renderer2d_get_viewport(int *o_x, int *o_y, int *o_w, int *o_h)
{
    struct sf_rect *vp = sf_array_tail(&renderer2d.viewports);
#define PTR_ASSIGN(ptr, val) do { if (ptr) { (*ptr) = val; } } while (0)
    PTR_ASSIGN(o_x, vp->x);
    PTR_ASSIGN(o_y, vp->y);
    PTR_ASSIGN(o_w, vp->w);
    PTR_ASSIGN(o_h, vp->h);
}
