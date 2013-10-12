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
    static int isreplay = 0;
    static double ntoreplay = 0;

    handle_mouse_button_right();
    if (g_app.im->keys[KEY_1] == KEY_PRESS) {
        canvas_record_save(g_app.upp->canvas, "record/tmp.record");
    } else if (g_app.im->keys[KEY_2] == KEY_PRESS) {
        canvas_record_load(g_app.upp->canvas, "record/tmp.record");
    } else if (g_app.im->keys[KEY_3] == KEY_PRESS) {
        isreplay = !isreplay;
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
    if (isreplay && canvas_record_canredo(g_app.upp->canvas)) {
        int n;
        ntoreplay += 512 * dt;
        n = ntoreplay;
        if (n > 0) {
            canvas_record_redo_n(g_app.upp->canvas, n);
            ntoreplay -= n;
        }
    }

    ui_manager_update(g_app.uim, g_app.im, dt);

    static int cnt = 0;
    static double elapse = 0;

    ++cnt;
    elapse += dt;
    if (elapse > 1.0) {
        dprintf("FPS: %d\n", cnt);
        cnt = 0;
        elapse -= 1.0;
    }
}

static void render(void) {
    ui_manager_render(g_app.uim, g_app.renderer2d);
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

    glfwSwapInterval(0);

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
    uint64_t cur_tick, last_tick;

    if (app_init(argc, argv) != 0) {
        return -1;
    }

    cur_tick = sf_get_ticks();
    while (window_isopen(g_app.window)) {
        last_tick = cur_tick;
        cur_tick = sf_get_ticks();

        input_manager_update(g_app.im);

        update((cur_tick - last_tick) * 1.0 / 1E9);

        render();

        glfwSwapBuffers(g_app.window->handle);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
