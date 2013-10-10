#include <stdio.h>
#include <unistd.h>

#include <sf_utils.h>
#include <sf_debug.h>

#include "app.h"

struct app g_app;

static char *WINDOW_TITLE = "Easy Paint";
static int WINDOW_WIDTH = 360;
static int WINDOW_HEIGHT = 600;


static void init(void) {
    g_app.upp = user_paint_panel_create(g_app.window->w, g_app.window->h);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.upp);
}

static void resize(struct window *win, int w, int h) {
    renderer2d_resize(g_app.renderer2d, w, h);

    user_paint_panel_resize(g_app.upp, w, h);
}

static void handle_mouse_button_right(void) {
    static int lastx, lasty;

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_PRESS) {
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_REPEAT
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        canvas_offset(g_app.upp->canvas, lastx - g_app.im->mouse.x,
                      lasty - g_app.im->mouse.y);
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

}

static void update(double dt) {
    handle_mouse_button_right();
    if (g_app.im->keys[KEY_1] == KEY_PRESS) {
        canvas_record_save(g_app.upp->canvas, "record/tmp.record");
    } else if (g_app.im->keys[KEY_2] == KEY_PRESS) {
        canvas_record_load(g_app.upp->canvas, "record/tmp.record");
    }
#if 0
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
    ui_manager_update(g_app.uim, g_app.im, dt);
}

static void render(void) {
    static uint64_t ticks = 0;
    static uint64_t elapse = 0;
    static uint64_t cnt = 0;
    static uint64_t totalticks = 0;

    if (ticks == 0) {
        ticks = sf_get_ticks();
    } else {
        elapse += sf_get_ticks() - ticks;
        ticks = sf_get_ticks();
    }

    ui_manager_render(g_app.uim, g_app.renderer2d);

    totalticks += sf_get_ticks() - ticks;
    ++cnt;

    if (elapse > 4E9) {
        dprintf("render costs %"PRIu64" ns/frame.\n",
                (uint64_t) (totalticks * 1.0f / cnt));
        cnt = 0;
        totalticks = 0;
        elapse -= 4E9;
    }
}

static int app_init(int argc, char *argv[]) {
    GLenum err;

    if (sf_init(argc, argv)) {
        return -1;
    }
#if 0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    g_app.window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_app.window) {
        glfwTerminate();
        return -1;
    }
    window_on_resize(g_app.window, resize);

    fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    /*
     * GLEW has a problem with core contexts.
     * It calls glGetString(GL_EXTENSIONS), which causes GL_INVALID_ENUM
     * on GL 3.2+ core context as soon as glewInit() is called.
     * It also doesn't fetch the function pointers.
     */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return -1;
    }
    while ((err = glGetError()) != GL_NO_ERROR);

    g_app.im = input_manager_create(g_app.window);
    g_app.renderer2d = renderer2d_create(g_app.window->w, g_app.window->h);
    g_app.uim = ui_manager_create();

    init();

    return 0;
}

int main(int argc, char *argv[]) {
    if (app_init(argc, argv) != 0) {
        return -1;
    }

    while (window_isopen(g_app.window)) {
        input_manager_update(g_app.im);

        update(0.0f);

        render();

        glfwSwapBuffers(g_app.window->handle);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
