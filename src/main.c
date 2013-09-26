#include <sf_utils.h>
#include <sf_debug.h>

#include "app.h"
#include "brush.h"


static struct brush *pencil;


void init(void) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    g_app.canvas = canvas_create(texture_load_2d("backgrounds/tex0.jpg"),
                                 0, 0, g_app.window->w, g_app.window->h);

    pencil = brush_pencil_create();
}

static void handle_mouse_button1(void) {
    static int ispressd = 0;
    static int lastx, lasty;

    if (ispressd == 0 && g_app.im->mouse.mb1.state == KEY_PRESS) {
        ispressd = 1;
        lastx = g_app.im->mouse.mb1.x;
        lasty = g_app.im->mouse.mb1.y;
    } else if (g_app.im->mouse.mb1.state == KEY_RELEASE) {
        ispressd = 0;
    }

    if (ispressd
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        if (sf_rect_iscontain(&g_app.canvas->viewport,
                              g_app.im->mouse.x, g_app.im->mouse.y)) {
            brush_drawline(pencil, g_app.canvas, lastx, lasty,
                           g_app.im->mouse.x,
                           g_app.im->mouse.y);
        }
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }
}

static void handle_mouse_button2(void) {
    static int ispressed = 0;
    static int lastx, lasty;

    if (ispressed == 0 && g_app.im->mouse.mb2.state == KEY_PRESS) {
        ispressed = 1;
        lastx = g_app.im->mouse.mb2.x;
        lasty = g_app.im->mouse.mb2.y;
    } else if (g_app.im->mouse.mb2.state == KEY_RELEASE) {
        ispressed = 0;
    }

    if (ispressed
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        canvas_offset(g_app.canvas, lastx - g_app.im->mouse.x,
                      lasty - g_app.im->mouse.y);
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

}

void update(double dt) {
    handle_mouse_button1();
    handle_mouse_button2();
}

void render(void) {
    uint64_t ticks;
    static uint64_t cnt = 0;
    static uint64_t totalticks = 0;

    ticks = sf_get_ticks();
    canvas_draw(g_app.canvas);

    totalticks += sf_get_ticks() - ticks;
    ++cnt;

    if (cnt > 10000) {
        dprintf("canvas_draw costs %"PRIu64" ns/frame.\n",
                (uint64_t) (totalticks * 1.0f / cnt));
        cnt = 0;
        totalticks = 0;
    }
}
