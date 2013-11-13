#ifndef UI_MENU_H
#define UI_MENU_H


#include <stdint.h>

#include <sf/list.h>

#include "ui.h"

struct ui_imagebox;


struct ui_menu {
    struct ui           ui;         /* inherit from ui, so this must be
                                     * the first element of the struct  */
    sf_list_t           items;

    uint8_t             background_color[4];
};

struct ui_menu *ui_menu_create(int w, int h);

void ui_menu_set_background_color(struct ui_menu *menu, uint8_t r, uint8_t g,
                                  uint8_t b, uint8_t a);

void ui_menu_add_item(struct ui_menu *menu, struct ui *ui);


#endif /* UI_MENU_H */
