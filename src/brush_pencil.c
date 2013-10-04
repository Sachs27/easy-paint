#include <stdlib.h>
#include <GL/glew.h>

#include "brush.h"


static void pencil_blend(struct canvas *canvas, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    float   alpha;
    uint8_t dst_color[4];
    uint8_t color[4];

    canvas_pick(canvas, x, y, dst_color, dst_color + 1, dst_color + 2,
                dst_color + 3);

    alpha = dst_color[3] / 255.0f;
    color[0] = r * alpha + dst_color[0] * (1 - alpha);
    color[1] = g * alpha + dst_color[1] * (1 - alpha);
    color[2] = b * alpha + dst_color[2] * (1 - alpha);
    alpha = a + dst_color[3];
    color[3] = alpha >= 255.0f ? (uint8_t) 255 : (uint8_t) alpha;

    /* no need to plot the same color */
    if (color[0] == dst_color[0] && color[1] == dst_color[1]
        && color[2] == dst_color[2] && color[3] == dst_color[3]) {
        return;
    }
    canvas_plot(canvas, x, y, color[0], color[1], color[2], color[3]);
}

static void pencil_plot(struct brush *brush, struct canvas *canvas,
                        int px, int py) {
    uint8_t r = brush->color[0];
    uint8_t g = brush->color[1];
    uint8_t b = brush->color[2];
    uint8_t a0 = brush->color[3];
    uint8_t a1 = (a0 / 255.0f) * (a0 / 255.0f) * 255;
    uint8_t a2 = (a1 / 255.0f) * (a1 / 255.0f) * 255;

    switch (rand() % 5) {
    case 0:
        /*
         *  o x
         *  x x
         */
        pencil_blend(canvas, px, py, r, g, b, a0);
        pencil_blend(canvas, px + 1, py, r, g, b, a1);
        pencil_blend(canvas, px, py + 1, r, g, b, a1);
        pencil_blend(canvas, px + 1, py + 1, r, g, b, a2);
        break;
    case 1:
        /*
         * x o
         * x x
         */
        pencil_blend(canvas, px, py, r, g, b, a1);
        pencil_blend(canvas, px + 1, py, r, g, b, a0);
        pencil_blend(canvas, px, py + 1, r, g, b, a2);
        pencil_blend(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 2:
        /*
         * x x
         * o x
         */
        pencil_blend(canvas, px, py, r, g, b, a1);
        pencil_blend(canvas, px + 1, py, r, g, b, a2);
        pencil_blend(canvas, px, py + 1, r, g, b, a0);
        pencil_blend(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 3:
        /*
         * x x
         * x o
         */
        pencil_blend(canvas, px, py, r, g, b, a2);
        pencil_blend(canvas, px + 1, py, r, g, b, a1);
        pencil_blend(canvas, px, py + 1, r, g, b, a1);
        pencil_blend(canvas, px + 1, py + 1, r, g, b, a0);
        break;
    case 4:
        /*
         *   x
         * x o x
         *   x
         */
        pencil_blend(canvas, px, py - 1, r, g, b, a1);
        pencil_blend(canvas, px - 1, py, r, g, b, a1);
        pencil_blend(canvas, px, py, r, g, b, a0);
        pencil_blend(canvas, px + 1, py, r, g, b, a1);
        pencil_blend(canvas, px, py + 1, r, g, b, a1);
        break;
    }
}


struct brush *brush_pencil_create(void) {
    struct brush *pencil;

    pencil = malloc(sizeof(*pencil));
    pencil->plot = pencil_plot;
    brush_set_color(pencil, 0, 0, 0, 64);
    brush_set_icon(pencil, texture_load_2d("res/icons/pencil.png"));

    return pencil;
}
