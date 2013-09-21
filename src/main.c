#include <stdio.h>
#include <sf_utils.h>
#include "app.h"
#include "brush.h"


static struct brush *pencil;


void init(void) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    /*glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);*/
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glDrawBuffer(GL_FRONT);

    g_app.canvas = canvas_create(NULL, 0, 0, 512, 512);

    pencil = brush_pencil_create();
    brush_set_color(pencil, 0.0f, 0.0f, 0.0f, 0.2f);
}

void update(double dt) {
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
            int dx, dy;

            dx = g_app.canvas->offset.x - g_app.canvas->viewport.x;
            dy = g_app.canvas->offset.y - g_app.canvas->viewport.y;

            brush_drawline(pencil, lastx + dx, lasty + dy,
                           g_app.im->mouse.x + dx,
                           g_app.im->mouse.y + dx);
        }
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }
}

void render(void) {
    uint64_t ticks;
    static int cnt = 0;
    static int totalticks = 0;

    ticks = sf_get_ticks();
    canvas_draw(g_app.canvas);

    totalticks += sf_get_ticks() - ticks;
    ++cnt;

    if (cnt > 100) {
        fprintf(stdout, "canvas_draw costs %"PRIu64" ns/frame.\n", (uint64_t) (totalticks * 1.0f / cnt));
        cnt = 0;
        totalticks = 0;
    }
}
