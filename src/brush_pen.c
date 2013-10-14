#include <stdlib.h>

#include "brush.h"
#include "canvas.h"


static void pen_blend(struct canvas *canvas, int x, int y,
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

static void pen_plot(struct brush *brush, struct canvas *canvas,
                           int px, int py) {
    /*
     * z x z
     * x o x
     * z x z
     */
    uint8_t r = brush->color[0];
    uint8_t g = brush->color[1];
    uint8_t b = brush->color[2];
    uint8_t a0 = brush->color[3];
    uint8_t a1 = a0 * 0.6;
    uint8_t a2 = a0 * 0.3;

    pen_blend(canvas, px - 1, py - 1, r, g, b, a2);
    pen_blend(canvas, px, py - 1, r, g, b, a1);
    pen_blend(canvas, px + 1, py - 1, r, g, b, a2);
    pen_blend(canvas, px - 1, py, r, g, b, a1);
    pen_blend(canvas, px, py, r, g, b, a0);
    pen_blend(canvas, px + 1, py, r, g, b, a1);
    pen_blend(canvas, px - 1, py + 1, r, g, b, a2);
    pen_blend(canvas, px, py + 1, r, g, b, a1);
    pen_blend(canvas, px + 1, py + 1, r, g, b, a2);

}

struct brush *brush_pen_create(void) {
    struct brush *pen;

    pen = malloc(sizeof(*pen));
    pen->plot = pen_plot;
    brush_set_color(pen, 0, 0, 0, 255);
    brush_set_icon(pen, texture_load_2d("res/icons/pen.png"));

    return pen;
}
