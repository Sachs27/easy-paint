#include <sf/utils.h>

#include "ui_menu.h"


static void ui_menu_update_items(struct ui_menu *menu) {
    int y = 0;
    sf_list_iter_t iter;

    if (sf_list_begin(&menu->items, &iter)) do {
        struct ui *ui = *(struct ui **) sf_list_iter_elt(&iter);
        ui->area.x = 0;
        ui->area.y = y;
        y += ui->area.h;
    } while (sf_list_iter_next(&iter));
}

static void ui_menu_on_render(struct ui_menu *menu, struct renderer2d *r) {
    int y = 0;
    sf_list_iter_t iter;

    renderer2d_fill_rect(r, 0, 0, menu->ui.area.w, menu->ui.area.h,
                         menu->background_color[0],
                         menu->background_color[1],
                         menu->background_color[2],
                         menu->background_color[3]);

    if (sf_list_begin(&menu->items, &iter)) do {
        struct ui *item = *(struct ui **) sf_list_iter_elt(&iter);
        if (y == 0) {
            y = item->area.h;
        }
        renderer2d_draw_line(r, 2,
                             0, y, item->area.w, y,
                             menu->background_color[0] / 2,
                             menu->background_color[1] / 2,
                             menu->background_color[2] / 2,
                             menu->background_color[3]);
        y += item->area.h;
    } while (sf_list_iter_next(&iter));
}

static void ui_menu_on_press(struct ui_menu *menu, int n, int x[n], int y[n]) {
    /*
     * Just ignore the press event.
     */
}


struct ui_menu *ui_menu_create(int w, int h) {
    struct ui_menu *menu;
    sf_list_def_t   def;

    menu = sf_alloc(sizeof(*menu));
    ui_init((struct ui *) menu, w, h);

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct ui *);
    sf_list_init(&menu->items, &def);

    UI_CALLBACK(menu, render, ui_menu_on_render);
    UI_CALLBACK(menu, press, ui_menu_on_press);

    return menu;
}

void ui_menu_set_background_color(struct ui_menu *menu, uint8_t r, uint8_t g,
                                  uint8_t b, uint8_t a) {
    menu->background_color[0] = r;
    menu->background_color[1] = g;
    menu->background_color[2] = b;
    menu->background_color[3] = a;
}

void ui_menu_add_item(struct ui_menu *menu, struct ui *item) {
    sf_list_push(&menu->items, &item);
    ui_add_child((struct ui *) menu, item, 0, 0);
    ui_menu_update_items(menu);
}
