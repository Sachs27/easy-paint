#ifndef UI_H
#define UI_H


#include <sf_list.h>
#include "sf_rect.h"

#include "input_manager.h"
#include "renderer2d.h"


struct ui;
struct ui_manager;

typedef void (ui_on_update_t)(struct ui *, struct input_manager *, double);

/**
 * Callback function called when ui is drawing.
 *
 * NOTE: the OpenGL's viewport has set to the ui's area.
 */
typedef void (ui_on_render_t)(struct ui *, struct renderer2d *);

/**
 * Callback function called when user press in the ui's area.
 *
 * (x, y) is the pressed point which is in the ui's coordinate.
 */
typedef void (ui_on_press_t)(struct ui *ui, int n, int x[n], int y[n]);

typedef void (ui_on_release_t)(struct ui *ui);

/**
 * Callback function called when the ui is pushing to the manager.
 */
typedef void (ui_on_push_t)(struct ui *ui, struct ui_manager *uim, int x, int y);

typedef void (ui_on_show_t)(struct ui *ui);

typedef void (ui_on_hide_t)(struct ui *ui);

enum ui_state {
    UI_STATE_HIDE,
    UI_STATE_SHOW,
};

struct ui {
    enum ui_state       state;
    struct sf_rect      area;

#define UI_CB_DEC(e) ui_on_ ## e ## _t *on_ ## e
    UI_CB_DEC(update);
    UI_CB_DEC(render);
    UI_CB_DEC(press);
    UI_CB_DEC(release);
    UI_CB_DEC(push);
    UI_CB_DEC(show);
    UI_CB_DEC(hide);
#undef UI_CB_DEC
};


int ui_init(struct ui *ui, int w, int h);

inline static void ui_show(struct ui *ui) {
    ui->state = UI_STATE_SHOW;
    if (ui->on_show) {
        ui->on_show(ui);
    }
}

inline static void ui_hide(struct ui *ui) {
    ui->state = UI_STATE_HIDE;
    if (ui->on_hide) {
        ui->on_hide(ui);
    }
}

/**
 * Register callback function for the ui's event.
 *
 * Available event value are:
 *    update
 *    render
 *    press
 *    release
 *    push
 *
 * @p pointer of the struct ui or it's child
 * @e event the callback function will register for
 * @func callback function
 */
#define UI_CALLBACK(p, e, func) do {                            \
    ((struct ui *) (p))->on_ ## e = (ui_on_ ## e ## _t *) func; \
} while (0)


/* ======================================================================= */


struct ui_manager {
    struct sf_list     *uis;        /* elt: (struct ui *) */
    int                 ispressed;  /* only one ui can be pressed
                                     * at one time.  */
    struct ui          *ui_pressed;
};


struct ui_manager *ui_manager_create(void);

void ui_manager_update(struct ui_manager *uim, struct input_manager *im,
                       double dt);

void ui_manager_render(struct ui_manager *uim, struct renderer2d *r);

/**
 * (x, y) is the position of the ui's upper-left corner which is in the
 * screen coordinate.
 */
void ui_manager_push(struct ui_manager *uim, int x, int y, struct ui *ui);


#endif /* UI_H */
