#ifndef UI_H
#define UI_H


#include <sf_list.h>
#include "sf_rect.h"

#include "input_manager.h"
#include "renderer2d.h"


struct ui;

typedef void (ui_on_update_t)(struct ui *, struct input_manager *, double);

/* Callback function called when ui is drawing.
 *
 * NOTE: the OpenGL's viewport has set to the ui's area.  */
typedef void (ui_on_render_t)(struct ui *, struct renderer2d *);

/* Callback function called when user press in the ui's area.
 *
 * (x, y) is the pressed point which is in the ui's coordinate.  */
typedef void (ui_on_press_t)(struct ui *ui, int n, int x[n], int y[n]);

typedef void (ui_on_release_t)(struct ui *ui);


struct ui {
    struct sf_rect      area;

    ui_on_update_t     *on_update;
    ui_on_render_t     *on_render;
    ui_on_press_t      *on_press;
    ui_on_release_t    *on_release;
};


void ui_init(struct ui *ui, int w, int h);

inline static void ui_on_update(struct ui *ui, ui_on_update_t *update_cb) {
    ui->on_update = update_cb;
}

inline static void ui_on_render(struct ui *ui, ui_on_render_t *render_cb) {
    ui->on_render = render_cb;
}

inline static void ui_on_press(struct ui *ui, ui_on_press_t *press_cb) {
    ui->on_press = press_cb;
}

inline static void ui_on_release(struct ui *ui, ui_on_release_t *release_cb) {
    ui->on_release = release_cb;
}


/* ======================================================================= */


struct ui_manager {
    struct input_manager   *im;
    struct renderer2d      *renderer2d;

    struct sf_list     *uis;        /* elt: (struct ui *) */
    int                 ispressed;  /* only one ui can be pressed
                                     * at one time.  */
    struct ui          *ui_pressed;
};


struct ui_manager *ui_manager_create(struct input_manager *im,
                                     struct renderer2d *r);

void ui_manager_update(struct ui_manager *uim, double dt);

void ui_manager_render(struct ui_manager *uim);

/* (x, y) is the position of the ui's upper-left corner which is in the
 * screen coordinate. */
void ui_manager_push(struct ui_manager *uim, int x, int y, struct ui *ui);


#endif /* UI_H */
