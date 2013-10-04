#include <stdlib.h>

#include "ui_imagebox.h"


static void ui_imagebox_on_render(struct ui_imagebox *ib,
                                  struct renderer2d *r) {
    renderer2d_draw_texture(r, 0, 0, 0, 0, ib->image, 0, 0, 0, 0);
}


struct ui_imagebox *ui_imagebox_create(int w, int h, struct texture *img) {
    struct ui_imagebox *ib;

    ib = malloc(sizeof(*ib));
    if (w == 0) {
        w = img->w;
    }
    if (h == 0) {
        h = img->h;
    }
    ui_init(&ib->ui, w, h);
    ui_on_render(&ib->ui, (ui_on_render_t *) ui_imagebox_on_render);
    ib->image = img;

    return ib;
}
