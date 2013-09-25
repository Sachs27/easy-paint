#include <assert.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "load_shaders.h"
#include "brush.h"


static void pencil_plot(struct brush *brush, struct canvas *canvas,
                        int px, int py) {
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
        canvas_plot(canvas, px, py, r, g, b, a0);
        canvas_plot(canvas, px + 1, py, r, g, b, a1);
        canvas_plot(canvas, px, py + 1, r, g, b, a1);
        canvas_plot(canvas, px + 1, py + 1, r, g, b, a2);
        break;
    case 1:
        /*
         * x o
         * x x
         */
        canvas_plot(canvas, px, py, r, g, b, a1);
        canvas_plot(canvas, px + 1, py, r, g, b, a0);
        canvas_plot(canvas, px, py + 1, r, g, b, a2);
        canvas_plot(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 2:
        /*
         * x x
         * o x
         */
        canvas_plot(canvas, px, py, r, g, b, a1);
        canvas_plot(canvas, px + 1, py, r, g, b, a2);
        canvas_plot(canvas, px, py + 1, r, g, b, a0);
        canvas_plot(canvas, px + 1, py + 1, r, g, b, a1);
        break;
    case 3:
        /*
         * x x
         * x o
         */
        canvas_plot(canvas, px, py, r, g, b, a2);
        canvas_plot(canvas, px + 1, py, r, g, b, a1);
        canvas_plot(canvas, px, py + 1, r, g, b, a1);
        canvas_plot(canvas, px + 1, py + 1, r, g, b, a0);
        break;
    case 4:
        /*
         *   x
         * x o x
         *   x
         */
        canvas_plot(canvas, px, py - 1, r, g, b, a1);
        canvas_plot(canvas, px - 1, py, r, g, b, a1);
        canvas_plot(canvas, px, py, r, g, b, a0);
        canvas_plot(canvas, px + 1, py, r, g, b, a1);
        canvas_plot(canvas, px, py + 1, r, g, b, a1);
        break;
    }
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
    pencil->plot = pencil_plot;

    return pencil;
}
