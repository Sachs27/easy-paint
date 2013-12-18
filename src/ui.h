#ifndef UI_H
#define UI_H


#include <sf/list.h>
#include "sf_rect.h"

#include "input_manager.h"

struct ui;
struct ui_manager;


typedef void (ui_on_update_t)(struct ui *, struct input_manager *, double);

/**
 * Callback function called when ui is drawing.
 *
 * NOTE: the OpenGL's viewport has set to the ui's area.
 */
typedef void (ui_on_render_t)(struct ui *);

/**
 * Callback function called when user press in the ui's area.
 *
 * (x, y) is the pressed point which is in the ui's coordinate.
 *
 * @return whether the press event will pass to parent ?
 *         0     - no
 *         other - yes
 */
typedef int (ui_on_press_t)(struct ui *ui, int x, int y);

typedef int (ui_on_long_press_t)(struct ui *ui, int x, int y);

typedef int (ui_on_tap_t)(struct ui *ui, int x, int y);

typedef void (ui_on_release_t)(struct ui *ui);

typedef void (ui_on_show_t)(struct ui *ui);

typedef void (ui_on_hide_t)(struct ui *ui);

typedef void (ui_on_resize_t)(struct ui *ui, int w, int h);

typedef void (ui_on_destroy_t)(struct ui *ui);

enum ui_state {
    UI_STATE_HIDE,
    UI_STATE_SHOW,
};

struct ui {
    enum ui_state   state;
    struct sf_rect  area;

    struct ui      *parent;

    sf_list_t      *childen; /* elt: (struct ui *) */

#define UI_CB_DEC(e) ui_on_ ## e ## _t *on_ ## e
    UI_CB_DEC(update);
    UI_CB_DEC(render);
    UI_CB_DEC(press);
    UI_CB_DEC(long_press);
    UI_CB_DEC(tap);
    UI_CB_DEC(release);
    UI_CB_DEC(show);
    UI_CB_DEC(hide);
    UI_CB_DEC(resize);
    UI_CB_DEC(destroy);
#undef UI_CB_DEC
};


int ui_init(struct ui *ui, int w, int h);

void ui_destroy(struct ui *ui);

/**
 * (x, y) is the position of the ui's upper-left corner which is in the
 * parent's coordinates.
 */
int ui_add_child(struct ui *ui, struct ui *child, int x, int y);

void ui_remove_child(struct ui *ui, struct ui *child);

void ui_show(struct ui *ui);

void ui_hide(struct ui *ui);

void ui_move(struct ui *ui, int x, int y);

void ui_resize(struct ui *ui, int w, int h);

void ui_get_screen_pos(struct ui *ui, int *o_x, int *o_y);

/**
 * Register callback function for the ui's event.
 *
 * @p pointer of the struct ui or it's child
 * @e event the callback function will register for
 * @func callback function
 */
#define UI_CALLBACK(p, e, func) do {                            \
    ((struct ui *) (p))->on_ ## e = func; \
} while (0)


/* ======================================================================= */

struct ui_manager {
    struct ui  *ui_pressed; /* only one ui can be
                             * pressed at one time.  */
    struct ui  *root;
};


int ui_manager_init(struct ui_manager *uim);

void ui_manager_destroy(struct ui_manager *uim);

void ui_manager_update(struct ui_manager *uim, struct input_manager *im,
                       double dt);

void ui_manager_render(struct ui_manager *uim);


void ui_manager_set_root(struct ui_manager *uim, struct ui *root);


#endif /* UI_H */
