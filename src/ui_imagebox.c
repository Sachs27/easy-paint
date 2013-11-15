#include <sf/utils.h>

#include "ui_imagebox.h"
#include "texture.h"


static void ui_imagebox_on_render(struct ui *ui,
                                  struct renderer2d *r) {
    struct ui_imagebox *ib = (struct ui_imagebox *) ui;
    if (ib->image) {
        renderer2d_draw_texture(r, 0, 0, 0, 0, ib->image, 0, 0, 0, 0);
    }
}


int ui_imagebox_init(struct ui_imagebox *ib,
                     int w, int h, struct texture *img) {
    if (w == 0) {
        w = img->w;
    }
    if (h == 0) {
        h = img->h;
    }

    ui_init((struct ui *) ib, w, h);
    ib->image = img;

    UI_CALLBACK(ib, render, ui_imagebox_on_render);

    return 0;
}

void ui_imagebox_set_image(struct ui_imagebox *ib, struct texture *img) {
    ib->image = img;
}
