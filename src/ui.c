#include <string.h>

#include <sf/utils.h>

#include "ui.h"


int ui_init(struct ui *ui, int w, int h) {
    sf_list_def_t def;

    memset(ui, 0, sizeof(*ui));

    ui->state = UI_STATE_HIDE;
    ui->area.x = 0;
    ui->area.y = 0;
    ui->area.w = w;
    ui->area.h = h;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct ui *);
    sf_list_init(&ui->childs, &def);

    return 0;
}

int ui_add_child(struct ui *ui, struct ui *child, int x, int y) {
    child->area.x = x;
    child->area.y = y;
    child->parent = ui;
    sf_list_push(&ui->childs, &child);
    return 0;
}

void ui_show(struct ui *ui) {
    sf_list_iter_t iter;

    ui->state = UI_STATE_SHOW;

    if (sf_list_begin(&ui->childs, &iter)) do {
        ui_show(*(struct ui **) sf_list_iter_elt(&iter));
    } while (sf_list_iter_next(&iter));

    if (ui->on_show) {
        ui->on_show(ui);
    }
}

void ui_hide(struct ui *ui) {
    sf_list_iter_t iter;

    if (sf_list_begin(&ui->childs, &iter)) do {
        ui_hide(*(struct ui **) sf_list_iter_elt(&iter));
    } while (sf_list_iter_next(&iter));

    ui->state = UI_STATE_HIDE;
    if (ui->on_hide) {
        ui->on_hide(ui);
    }
}

void ui_move(struct ui *ui, int x, int y) {
    ui->area.x = x;
    ui->area.y = y;
}

void ui_resize(struct ui *ui, int w, int h) {
    ui->area.w = w;
    ui->area.h = h;
    if (ui->on_resize) {
        ui->on_resize(ui, w, h);
    }
}

void ui_get_screen_pos(struct ui *ui, int *o_x, int *o_y) {
    int x = 0;
    int y = 0;
    while (ui) {
        x += ui->area.x;
        y += ui->area.y;
        ui = ui->parent;
    }
    if (o_x) {
        *o_x = x;
    }
    if (o_y) {
        *o_y = y;
    }
}

/* ======================================================================= */

struct ui_manager *ui_manager_create(void) {
    struct ui_manager  *uim;
    sf_list_def_t       def;

    uim = sf_alloc(sizeof(*uim));

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct ui *);
    sf_list_init(&uim->uis, &def);

    uim->ui_pressed = NULL;
    uim->root = NULL;

    return uim;
}

/**
 * @return whether the event will pass to parent ?
 *         0     - no
 *         other - yes
 */
static int handle_press_event(struct ui_manager *uim, struct ui *ui,
                              int n, int *x, int *y) {
    int             i;
    sf_array_def_t  def;
    sf_array_t      ax, ay;
    sf_list_iter_t  iter;
    int             ispass = 1;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(int);
    sf_array_init(&ax, &def);
    sf_array_init(&ay, &def);

    if (sf_list_rbegin(&ui->childs, &iter)) do {
        struct ui *ui = *(struct ui**) sf_list_iter_elt(&iter);

        if (ui->state != UI_STATE_SHOW) {
            continue;
        }
        sf_array_clear(&ax);
        sf_array_clear(&ay);

        for (i = 0; i < n; ++i) {
            if (sf_rect_iscontain(&ui->area, x[i], y[i])) {
                int tmp;
                tmp = x[i] - ui->area.x;
                sf_array_push(&ax, &tmp);
                tmp = y[i] - ui->area.y;
                sf_array_push(&ay, &tmp);
            }
        }

        if (sf_array_cnt(&ax) == 0) {
            continue;
        }

        ispass = handle_press_event(uim, ui, sf_array_cnt(&ax),
                                    sf_array_head(&ax), sf_array_head(&ay));
        if (!ispass) {
            break;
        }
    } while (sf_list_iter_next(&iter));

    if (ispass && ui->on_press) {
        ispass = ui->on_press(ui, n, x, y);

        /* for now, just notify the top ui for the press event */
        ispass = 0;
        uim->ui_pressed = ui;
    }

    sf_array_destroy(&ax);
    sf_array_destroy(&ay);

    return ispass;
}

static void update_ui(struct ui *ui, struct input_manager *im, double dt) {
    sf_list_iter_t iter;

    if (sf_list_begin(&ui->childs, &iter)) do {
        update_ui(*(struct ui**) sf_list_iter_elt(&iter), im, dt);
    } while (sf_list_iter_next(&iter));

    if (ui->state == UI_STATE_SHOW && ui->on_update) {
        ui->on_update(ui, im, dt);
    }
}

void ui_manager_update(struct ui_manager *uim, struct input_manager *im,
                       double dt) {
    if (uim->root && !uim->ui_pressed && uim->root->state == UI_STATE_SHOW
        && im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        int x, y;
        x = im->mouse.x - uim->root->area.x;
        y = im->mouse.y - uim->root->area.y;
        handle_press_event(uim, uim->root, 1, &x, &y);
    } else if (im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
        if (uim->ui_pressed) {
            if (uim->ui_pressed->on_release) {
                uim->ui_pressed->on_release(uim->ui_pressed);
            }
            uim->ui_pressed = NULL;
        }
    }

    if (uim->root) {
        update_ui(uim->root,im ,dt);
    }
}

static void render_ui(struct ui *ui, struct renderer2d *r) {
    int             x, y;
    sf_list_iter_t  iter;

    if (ui->state == UI_STATE_HIDE) {
        return;
    }

    renderer2d_get_viewport(r, &x, &y, NULL, NULL);
    x += ui->area.x;
    y += ui->area.y;
    renderer2d_push_viewport(r, x, y, ui->area.w, ui->area.h);
    if (ui->on_render) {
        ui->on_render(ui, r);
    }

    if (sf_list_begin(&ui->childs, &iter)) do {
        render_ui(*(struct ui **) sf_list_iter_elt(&iter), r);
    } while (sf_list_iter_next(&iter));

    renderer2d_pop_viewport(r);
}

void ui_manager_render(struct ui_manager *uim, struct renderer2d *r) {
    if (uim->root && uim->root->state != UI_STATE_HIDE) {
        render_ui(uim->root, r);
    }
}

void ui_manager_set_root(struct ui_manager *uim, struct ui *root) {
    uim->root = root;
}
