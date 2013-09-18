#include <assert.h>
#include <stdlib.h>

#include "brush.h"
#include "app.h"


static void pencil_plot(struct brush *brush, int x, int y) {
    int px = x - g_app.canvas->viewport.x;
    int py = y - g_app.canvas->viewport.y;
    scalar_t r = brush->color.r;
    scalar_t g = brush->color.g;
    scalar_t b = brush->color.b;
    scalar_t a0 = brush->color.a;
    scalar_t a1 = a0 * a0;
    scalar_t a2 = a1 * a1;

    switch (rand() % 5) {
    case 0:
        /*
         *  o x
         *  x x
         */
        canvas_set_pixel(g_app.canvas, 0, px, py, r, g, b, a0);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px, py + 1, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py + 1, r, g, b, a2);
        break;
    case 1:
        /*
         * x o
         * x x
         */
        canvas_set_pixel(g_app.canvas, 0, px, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py, r, g, b, a0);
        canvas_set_pixel(g_app.canvas, 0, px, py + 1, r, g, b, a2);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py + 1, r, g, b, a1);
        break;
    case 2:
        /*
         * x x
         * o x
         */
        canvas_set_pixel(g_app.canvas, 0, px, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py, r, g, b, a2);
        canvas_set_pixel(g_app.canvas, 0, px, py + 1, r, g, b, a0);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py + 1, r, g, b, a1);
        break;
    case 3:
        /*
         * x x
         * x o
         */
        canvas_set_pixel(g_app.canvas, 0, px, py, r, g, b, a2);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px, py + 1, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py + 1, r, g, b, a0);
        break;
    case 4:
        /*
         *   x
         * x o x
         *   x
         */
        canvas_set_pixel(g_app.canvas, 0, px, py - 1, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px - 1, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px, py, r, g, b, a0);
        canvas_set_pixel(g_app.canvas, 0, px + 1, py, r, g, b, a1);
        canvas_set_pixel(g_app.canvas, 0, px, py + 1, r, g, b, a1);
        break;
    }
}

struct brush *brush_pencil_create(void) {
    struct brush *pencil;

    pencil = malloc(sizeof(*pencil));
    assert(pencil != NULL);
    pencil->plot = pencil_plot;

    return pencil;
}
