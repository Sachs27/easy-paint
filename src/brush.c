#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>

#include "brush.h"
#include "load_shaders.h"


void brush_drawline(struct brush *brush, struct canvas *canvas,
                    int x0, int y0, int x1, int y1) {
#define int_swap(x, y) do {     \
    int __int_swap_tmp__ = x;   \
    x = y;                      \
    y = __int_swap_tmp__;       \
} while(0)

    int err;
    int x, y, ystep;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int steep = (dy - dx) > 0 ? 1 : 0;

    canvas_set_current_brush(canvas, brush);

    if (steep) {
        int_swap(x0, y0);
        int_swap(x1, y1);
    }

    if (x0 > x1) {
        int_swap(x0, x1);
        int_swap(y0, y1);
    }

    dx = x1 - x0;
    dy = abs(y1 - y0);
    err = dx / 2;
    y = y0;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = x0; x <= x1; ++x) {
        int px = x, py = y;
        if (steep) {
            px = y;
            py = x;
        }
        /* plot(px, py) */
        canvas_plot(canvas, px, py);

        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
#undef int_swap
}

void brush_set_color(struct brush *brush,
                     scalar_t r, scalar_t g, scalar_t b, scalar_t a) {
    brush->color.r = r;
    brush->color.g = g;
    brush->color.b = b;
    brush->color.a = a;
}

struct brush *brush_pencil_create(void) {
    static struct shader_info pencils[] = {
        {GL_VERTEX_SHADER, "brush/pencil.vs.glsl"},
        {GL_FRAGMENT_SHADER, "brush/pencil.fs.glsl"},
        {GL_NONE, NULL}
    };
    struct brush *pencil;

    pencil = malloc(sizeof(*pencil));
    assert(pencil != NULL);
    pencil->prog = load_shaders(pencils);
    pencil->point_size = 5;
    pencil->maxtype = 5;

    return pencil;
}
