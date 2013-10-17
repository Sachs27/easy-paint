#include <stdlib.h>

#include "ui_imagebox.h"
#include "texture.h"


static void ui_imagebox_on_render(struct ui_imagebox *ib,
                                  struct renderer2d *r) {
    if (ib->image) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, ib->image, 0, 0, 0, 0);
    }
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
    ib->image = img;

    UI_CALLBACK(ib, render, ui_imagebox_on_render);

    return ib;
}
