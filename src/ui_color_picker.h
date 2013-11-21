#ifndef UI_COLOR_PICKER_H
#define UI_COLOR_PICKER_H


#include "ui.h"


struct ui_color_picker {
    struct ui           ui;         /* inherit from ui, so this must be
                                     * the first element of the struct  */

    float               h, s, l;

    float               ncolor[3], ocolor[3];

    int                 ispress_lightness;
    struct texture      lightness;
    struct sf_rect      lightness_area;

    int                 ispress_circle;
    struct texture      circle;
    struct sf_rect      circle_area;
};


int ui_color_picker_init(struct ui_color_picker *cp, int w, int h);

void ui_color_picker_set_color(struct ui_color_picker *cp, float color[3]);

void ui_color_picker_get_color(struct ui_color_picker *cp, float color[3]);


#endif /* UI_COLOR_PICKER_H */
