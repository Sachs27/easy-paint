#ifndef UI_TOOLBOX_H
#define UI_TOOLBOX_H


#include <sf/list.h>

#include "ui.h"


struct ui_toolbox {
    struct ui   ui;                     /* inherit from ui, so this must be
                                         * the first element of the struct  */
    uint8_t     background_color[4];
    sf_list_t   buttons;                /* elt: (struct ui *) */
};


int ui_toolbox_init(struct ui_toolbox *tb, int w, int h,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 *  Add a button to the toolbox, this is valid only __before__ pushing the
 *  toolbox to ui manager.
 */
void ui_toolbox_add_button(struct ui_toolbox *tb, struct ui *ui);


#endif /* UI_TOOLBOX_H */
