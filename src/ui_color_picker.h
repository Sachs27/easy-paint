#ifndef UI_COLOR_PICKER_H
#define UI_COLOR_PICKER_H


#include "ui.h"


struct ui_color_picker {
    struct ui           ui;         /* inherit from ui, so this must be
                                     * the first element of the struct  */

    float               h, s, l;
    float               r, g, b;

    struct texture      lightness;
    struct sf_rect      lightness_area;

    struct texture      circle;
    struct sf_rect      circle_area;
};


int ui_color_picker_init(struct ui_color_picker *cp, int w, int h);



#endif /* UI_COLOR_PICKER_H */
