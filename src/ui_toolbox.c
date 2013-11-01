#include <stdlib.h>

#include "ui_toolbox.h"


static void ui_toolbox_update_buttons(struct ui_toolbox *tb) {
    float xstep;
    float x;

    /*
     * 假设每个 buttons 的宽度相同，高度和 toolbox 一样。
     * 后期改进可以使用权重来计算每个 button 的位置。
     */
    xstep = tb->ui.area.w * 1.0f / (tb->buttons->nelts + 1);

    x = xstep;

    SF_LIST_BEGIN(tb->buttons, struct ui *, pui);
        struct ui *button = *pui;
        ui_move(button, x - button->area.w / 2, 0);
        x += xstep;
    SF_LIST_END();
}

static void ui_toolbox_on_render(struct ui_toolbox *tb,
                                 struct renderer2d *r) {
    /* draw background */
    renderer2d_fill_rect(r, 0, 0, tb->ui.area.w, tb->ui.area.h,
                         tb->background_color[0],
                         tb->background_color[1],
                         tb->background_color[2],
                         tb->background_color[3]);

    renderer2d_draw_line(r, 2, 0, 0, tb->ui.area.w, 0,
                         tb->background_color[0] / 2,
                         tb->background_color[1] / 2,
                         tb->background_color[2] / 2,
                         tb->background_color[3]);
}

static void ui_toolbox_on_press(struct ui_toolbox *tb,
                                int n, int x[n], int y[n]) {
    /*
     * Just ignore the press event.
     */
}

static void ui_toolbox_on_resize(struct ui_toolbox *tb, int w, int h) {
    ui_toolbox_update_buttons(tb);
}


struct ui_toolbox *ui_toolbox_create(int w, int h, uint8_t r, uint8_t g,
                                     uint8_t b, uint8_t a) {
    struct ui_toolbox *tb;

    tb = malloc(sizeof(*tb));
    if (ui_toolbox_init(tb, w, h, r, g, b, a) != 0) {
        free(tb);
        return NULL;
    }
    return tb;
}

int ui_toolbox_init(struct ui_toolbox *tb, int w, int h,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    ui_init((struct ui *) tb, w, h);
    tb->background_color[0] = r;
    tb->background_color[1] = g;
    tb->background_color[2] = b;
    tb->background_color[3] = a;
    if ((tb->buttons = sf_list_create(sizeof(struct ui *))) == NULL) {
        return -1;
    }

    UI_CALLBACK(tb, render, ui_toolbox_on_render);
    UI_CALLBACK(tb, press, ui_toolbox_on_press);
    UI_CALLBACK(tb, resize, ui_toolbox_on_resize);

    return 0;
}

void ui_toolbox_add_button(struct ui_toolbox *tb, struct ui *ui) {
    sf_list_push(tb->buttons, &ui);
    ui_add_child((struct ui *) tb, ui, 0, 0);
    ui_toolbox_update_buttons(tb);
}
