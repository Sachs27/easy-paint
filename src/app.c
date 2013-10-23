#include <GL/glew.h>
#include <sf_utils.h>
#include <sf_debug.h>

#include "app.h"
#include "user_paint_panel.h"
#include "ui_replay_panel.h"
#include "ui_imagebox.h"
#include "ui_menu.h"
#include "texture.h"
#include "resource_manager.h"


struct app g_app;
static void menuicon_on_press(struct ui *ui, int n, int x[n], int y[n]) {
    ui_show((struct ui *) g_app.menu);
}

static void menu_on_press(struct ui *ui, int n, int x[n], int y[n]) {
    ui_hide((struct ui *) g_app.menu);
}

static void app_change_stage(int stage) {
    switch (stage) {
    case APP_STAGE_DOODLE:
        ui_hide((struct ui *) g_app.urp);
        ui_show((struct ui *) g_app.upp);
        break;
    case APP_STAGE_TEACHING:
        ui_hide((struct ui *) g_app.upp);
        ui_show((struct ui *) g_app.urp);
        break;
    }
}

static void menu_item_on_press(struct ui *ui, int n, int x[n], int y[n]) {
    int i = 0;
    SF_LIST_BEGIN(g_app.menu->items, struct ui *, p);
        struct ui *item = *p;
        if (item == ui) {
            app_change_stage(i);
            ui_hide((struct ui *) g_app.menu);
            return;
        }
        ++i;
    SF_LIST_END();
}

void app_load_resource(const char *rootpath) {
    int i;

    if (g_app.rm == NULL) {
        g_app.rm = resource_manager_create(rootpath);
    }

    for (i = 0; i < RESOURCE_NTEXTURES; ++i) {
        resource_manager_load(g_app.rm, RESOURCE_TEXTURE, i);
    }
}

int app_init(void) {
    g_app.renderer2d = renderer2d_create(g_app.window->w, g_app.window->h);
    g_app.uim = ui_manager_create();

    g_app.upp = user_paint_panel_create(g_app.window->w, g_app.window->h,
                                        g_app.rm);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.upp);

    g_app.urp = ui_replay_panel_create(g_app.window->w, g_app.window->h,
                                       g_app.rm);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.urp);

    g_app.menuicon = ui_imagebox_create(
        0, 0, resource_manager_get(g_app.rm, RESOURCE_TEXTURE,
                                   RESOURCE_TEXTURE_ICON_PARENT));
    UI_CALLBACK(g_app.menuicon, press, menuicon_on_press);
    ui_manager_push(g_app.uim, 0, g_app.window->h - g_app.menuicon->ui.area.h,
                    (struct ui *) g_app.menuicon);

    g_app.logo = ui_imagebox_create(
        0, 0, resource_manager_get(g_app.rm, RESOURCE_TEXTURE,
                                   RESOURCE_TEXTURE_ICON_LOGO));
    g_app.label1 = ui_imagebox_create(
        0, 0, resource_manager_get(g_app.rm, RESOURCE_TEXTURE,
                                   RESOURCE_TEXTURE_ICON_LABEL1));

    g_app.label2 = ui_imagebox_create(
        0, 0, resource_manager_get(g_app.rm, RESOURCE_TEXTURE,
                                   RESOURCE_TEXTURE_ICON_LABEL2));

    g_app.label3 = ui_imagebox_create(
        0, 0, resource_manager_get(g_app.rm, RESOURCE_TEXTURE,
                                   RESOURCE_TEXTURE_ICON_LABEL3));

    g_app.menu = ui_menu_create(256, g_app.window->h);
    UI_CALLBACK(g_app.menu, press, menu_on_press);
    ui_menu_set_background_color(g_app.menu, 64, 64, 64, 250);
    ui_menu_add_item(g_app.menu, (struct ui *) g_app.logo);
    ui_menu_add_item(g_app.menu, (struct ui *) g_app.label1);
    ui_menu_add_item(g_app.menu, (struct ui *) g_app.label2);
    ui_menu_add_item(g_app.menu, (struct ui *) g_app.label3);
    {
        int i = 0;
        SF_LIST_BEGIN(g_app.menu->items, struct ui *, p);
            struct ui *item = *p;
            if (i != 0) {
                UI_CALLBACK(item, press, menu_item_on_press);
            }
            ++i;
        SF_LIST_END();
    }
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.menu);
    ui_hide((struct ui *) g_app.menu);

    app_change_stage(APP_STAGE_DOODLE);

    return 0;
}

void app_on_resize(struct window *win, int w, int h) {
    renderer2d_resize(g_app.renderer2d, w, h);

    user_paint_panel_resize(g_app.upp, w, h);

    ui_replay_panel_resize(g_app.urp, w, h);

    g_app.menuicon->ui.area.y = h - g_app.menuicon->ui.area.h;
    g_app.menu->ui.area.h = h;
}

#if 0
static void handle_mouse_button_right(void) {
    static int lastx, lasty;

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_PRESS) {
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_REPEAT
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        canvas_offset(&g_app.upp->canvas, lastx - g_app.im->mouse.x,
                      lasty - g_app.im->mouse.y);
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

}
#endif

void app_on_update(double dt) {
#if 0
    handle_mouse_button_right();
    if (g_app.im->keys[KEY_UP] == KEY_PRESS) {
        int x, y;
        canvas_screen_to_canvas(g_app.canvas, g_app.im->mouse.x,
                                g_app.im->mouse.y, &x, &y);
        canvas_zoom_in(g_app.canvas, x, y);
    } else if (g_app.im->keys[KEY_DOWN] == KEY_PRESS) {
        int x, y;
        canvas_screen_to_canvas(g_app.canvas, g_app.im->mouse.x,
                                g_app.im->mouse.y, &x, &y);
        canvas_zoom_out(g_app.canvas, x, y);
    }
#endif
    static int cnt = 0;
    static double elapse = 0;

    ++cnt;
    elapse += dt;
    if (elapse > 1.0) {
        dprintf("FPS: %d\n", cnt);
        cnt = 0;
        elapse -= 1.0;
    }

    ui_manager_update(g_app.uim, g_app.im, dt);
}

void app_on_render(void) {
    ui_manager_render(g_app.uim, g_app.renderer2d);
}

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOGI(...)  printf(__VA_ARGS__)
#define  LOGE(...)  printf(__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    GLint error;
    for (error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] =
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char gFragmentShader[] =
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

int app_init(void) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return 0;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);
    return 0;
}

void app_on_resize(struct window *win, int w, int h) {
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };

void app_on_update(double dt) {
}

void app_on_render(void) {
    static float grey;
    grey += 0.01f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    glClearColor(grey, grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGlError("glDrawArrays");
}
#endif
