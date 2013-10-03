#include <sf_utils.h>
#include <sf_debug.h>

#include "app.h"
#include "brush.h"


static struct brush *pencil;
static struct brush *pen;
static struct brush *eraser;

static struct canvas *canvas2;

void init(void) {
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    g_app.canvas = canvas_create(NULL, 512, 480);
    canvas2 = canvas_create(NULL, 128, 128);

    ui_manager_push(g_app.uim, 48, 0, (struct ui *) g_app.canvas);
    ui_manager_push(g_app.uim, 600, 0, (struct ui *) canvas2);

    pencil = brush_pencil_create();
    pen = brush_pen_create();
    eraser = brush_eraser_create();

    g_app.cur_brush = pencil;
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

void update(double dt) {
    /*handle_mouse_button_left();*/
    handle_mouse_button_right();

    if (g_app.im->keys[KEY_1] == KEY_PRESS) {
        g_app.cur_brush = pencil;
    } else if (g_app.im->keys[KEY_2] == KEY_PRESS) {
        g_app.cur_brush = pen;
    } else if (g_app.im->keys[KEY_3] == KEY_PRESS) {
        g_app.cur_brush = eraser;
    }

    if (g_app.im->keys[KEY_LEFT] == KEY_PRESS) {
        canvas_record_undo(g_app.canvas);
    } else if (g_app.im->keys[KEY_RIGHT] == KEY_PRESS) {
        canvas_record_redo(g_app.canvas);
    } else if (g_app.im->keys[KEY_UP] == KEY_PRESS) {
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

    ui_manager_update(g_app.uim, dt);
}

void render(void) {
    uint64_t ticks;
    static uint64_t cnt = 0;
    static uint64_t totalticks = 0;

    ticks = sf_get_ticks();

    ui_manager_render(g_app.uim);

    totalticks += sf_get_ticks() - ticks;
    ++cnt;

    if (cnt > 10000) {
        dprintf("ui_manager_render costs %"PRIu64" ns/frame.\n",
                (uint64_t) (totalticks * 1.0f / cnt));
        cnt = 0;
        totalticks = 0;
    }
}
