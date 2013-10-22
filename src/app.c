#if defined(__WIN32__)
#include <windows.h>
#endif /* defined(__WIN32__) */

#include <stdio.h>
#include <unistd.h>

#include <sf_utils.h>
#include <sf_debug.h>

#include "sf.h"
#include "app.h"
#include "system.h"
#include "record.h"
#include "canvas.h"
#include "user_paint_panel.h"
#include "ui_replay_panel.h"
#include "ui_imagebox.h"
#include "ui_menu.h"
#include "texture.h"

struct app g_app;

static char *WINDOW_TITLE = "Easy Paint";
static int WINDOW_WIDTH = 480;
static int WINDOW_HEIGHT = 800;


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

static void init(void) {
    g_app.upp = user_paint_panel_create(g_app.window->w, g_app.window->h);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.upp);

    g_app.urp = ui_replay_panel_create(g_app.window->w, g_app.window->h);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.urp);

    g_app.menuicon = ui_imagebox_create(
                        0, 0, texture_load_2d("res/icons/parent.png"));
    UI_CALLBACK(g_app.menuicon, press, menuicon_on_press);
    ui_manager_push(g_app.uim, 0, g_app.window->h - g_app.menuicon->ui.area.h,
                    (struct ui *) g_app.menuicon);

    g_app.logo = ui_imagebox_create(0, 0,
                                    texture_load_2d("res/icons/logo.png"));

    g_app.label1 = ui_imagebox_create(0, 0,
                                      texture_load_2d("res/icons/label1.png"));

    g_app.label2 = ui_imagebox_create(0, 0,
                                      texture_load_2d("res/icons/label2.png"));

    g_app.label3 = ui_imagebox_create(0, 0,
                                      texture_load_2d("res/icons/label3.png"));

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

    app_change_stage(APP_STAGE_TEACHING);
}

static void resize(struct window *win, int w, int h) {
    renderer2d_resize(g_app.renderer2d, w, h);

    user_paint_panel_resize(g_app.upp, w, h);

    ui_replay_panel_resize(g_app.urp, w, h);

    g_app.menuicon->ui.area.y = h - g_app.menuicon->ui.area.h;
    g_app.menu->ui.area.h = h;
}

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

static void update(double dt) {
    handle_mouse_button_right();
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

#if defined(__WIN32__)
    typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(0);
#else
    glfwSwapInterval(0);
#endif /* defined(__WIN32__) */

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
