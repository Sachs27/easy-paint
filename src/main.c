#include "app.h"
#include "brush.h"


static struct brush *pencil;


void init(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDrawBuffer(GL_FRONT);

    g_app.canvas = canvas_create(texture_load_2d("backgrounds/canvas1_1.png"),
                                 0, 0, 512, 512);

    pencil = brush_pencil_create();
    brush_set_color(pencil, 0.0f, 0.0f, 0.0f, 1.0f);
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
            brush_drawline(pencil, lastx, lasty,
                           g_app.im->mouse.x, g_app.im->mouse.y);
        }
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }
}

void render(void) {
    canvas_draw(g_app.canvas);
}
