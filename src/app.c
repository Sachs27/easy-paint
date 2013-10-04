#include <stdio.h>
#include <unistd.h>

#include <sf_utils.h>
#include <sf_debug.h>

#include "app.h"
#include "brush.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"

struct app g_app;

static char *WINDOW_TITLE = "Easy Paint";
static int WINDOW_WIDTH = 360;
static int WINDOW_HEIGHT = 600;

static struct brush *pencil;
static struct brush *pen;
static struct brush *eraser;

static struct ui_toolbox  *toolbox;
static struct ui_imagebox *undo;
static struct ui_imagebox *brushbox;
static struct ui_imagebox *redo;

static void undo_on_press(struct ui_imagebox *undo,
                          int n, int x[n], int y[n]) {
    canvas_record_undo(g_app.canvas);
}

static void redo_on_press(struct ui_imagebox *redo,
                          int n, int x[n], int y[n]) {
    canvas_record_redo(g_app.canvas);
}

static void init(void) {
    g_app.canvas = canvas_create(g_app.window->w, g_app.window->h - 48);
    ui_manager_push(g_app.uim, 0, 0, (struct ui *) g_app.canvas);

    pencil = brush_pencil_create();
    pen = brush_pen_create();
    eraser = brush_eraser_create();

    g_app.canvas->cur_brush = pencil;

    toolbox = ui_toolbox_create(g_app.canvas->ui.area.w, 48,
                                200, 200, 200, 200);
    ui_manager_push(g_app.uim, g_app.canvas->ui.area.x,
                    g_app.canvas->ui.area.y + g_app.canvas->ui.area.h,
                    (struct ui *) toolbox);

    undo = ui_imagebox_create(0, 0, texture_load_2d("res/icons/undo.png"));
    ui_on_press((struct ui *) undo, (ui_on_press_t *) undo_on_press);
    brushbox = ui_imagebox_create(0, 0, g_app.canvas->cur_brush->icon);
    redo = ui_imagebox_create(0, 0, texture_load_2d("res/icons/redo.png"));
    ui_on_press((struct ui *) redo, (ui_on_press_t *) redo_on_press);

    ui_toolbox_add_button(toolbox, (struct ui *) undo);
    ui_toolbox_add_button(toolbox, (struct ui *) brushbox);
    ui_toolbox_add_button(toolbox, (struct ui *) redo);
}

static void resize(struct window *win, int w, int h) {
    renderer2d_resize(g_app.renderer2d,
                      g_app.window->w, g_app.window->h);

    canvas_resize(g_app.canvas, w, h - 48);

#define TOOLBOX_MIN_WIDTH 288
    if (w < TOOLBOX_MIN_WIDTH) {
        ui_toolbox_resize(toolbox, TOOLBOX_MIN_WIDTH, toolbox->ui.area.h);
    } else {
        ui_toolbox_resize(toolbox, w, toolbox->ui.area.h);
    }
    ui_toolbox_move(toolbox, g_app.canvas->ui.area.x,
                   g_app.canvas->ui.area.y + g_app.canvas->ui.area.h);
}

#if 0
static void handle_mouse_button_left(void) {
    static int lastx, lasty;

    if (g_app.im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
        canvas_record_begin(g_app.canvas);
    } else if (g_app.im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
        canvas_record_end(g_app.canvas);
    }

    if (g_app.im->keys[KEY_MB_LEFT] == KEY_REPEAT
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        if (sf_rect_iscontain(&g_app.canvas->ui.area,
                              g_app.im->mouse.x, g_app.im->mouse.y)) {
            int x0, y0, x1, y1;
            canvas_screen_to_canvas(g_app.canvas, lastx, lasty, &x0, &y0);
            canvas_screen_to_canvas(g_app.canvas, g_app.im->mouse.x,
                                                  g_app.im->mouse.y,
                                                  &x1, &y1);
            brush_drawline(g_app.cur_brush, g_app.canvas, x0, y0, x1, y1);
        }
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }
}
#endif

static void handle_mouse_button_right(void) {
    static int lastx, lasty;

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_PRESS) {
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_REPEAT
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        canvas_offset(g_app.canvas, lastx - g_app.im->mouse.x,
                      lasty - g_app.im->mouse.y);
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

}

static void update(double dt) {
    if (g_app.canvas->ui.area.w != g_app.window->w
        || g_app.canvas->ui.area.h != g_app.window->h) {
    }
    /*handle_mouse_button_left();*/
    handle_mouse_button_right();

    if (g_app.im->keys[KEY_1] == KEY_PRESS) {
        g_app.canvas->cur_brush = pencil;
    } else if (g_app.im->keys[KEY_2] == KEY_PRESS) {
        g_app.canvas->cur_brush = pen;
    } else if (g_app.im->keys[KEY_3] == KEY_PRESS) {
        g_app.canvas->cur_brush = eraser;
    }

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

    ui_imagebox_set_image(brushbox, g_app.canvas->cur_brush->icon);
    ui_manager_update(g_app.uim, dt);
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

    ui_manager_render(g_app.uim);

    /*renderer2d_draw_texture(g_app.renderer2d, 100, 0, tex->w, tex->h, tex, 0, 0, 0, 0);*/

    totalticks += sf_get_ticks() - ticks;
    ++cnt;

    if (elapse > 3E9) {
        dprintf("render costs %"PRIu64" ns/frame.\n",
                (uint64_t) (totalticks * 1.0f / cnt));
        cnt = 0;
        totalticks = 0;
        elapse -= 3E9;
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
    g_app.uim = ui_manager_create(g_app.im, g_app.renderer2d);

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
