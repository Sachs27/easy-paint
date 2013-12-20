#ifndef UI_SELECT_RECORD_H
#define UI_SELECT_RECORD_H


#include <sf/array.h>

#include "ui.h"
#include "ui_imagebox.h"


struct ui_select_record {
    struct ui           ui;         /* inherit from ui, so this must be
                                     * the first element of the struct  */

    sf_array_t          records;
    sf_array_t          textures;
    int                 texture_w, texture_h;
    int                 padding;
    int                 yoffset;

    int                 ispressed;
    int                 lastx, lasty;

    struct texture     *new_image;
    struct ui_imagebox  ib_new;
};


int ui_select_record_init(struct ui_select_record *sr, int w, int h);



#endif /* UI_SELECT_RECORD_H */
