#include <string.h>

#include <sf/log.h>
#include <sf/utils.h>

#include "ui.h"
#include "renderer2d.h"


int ui_init(struct ui *ui, int w, int h) {
    sf_memzero(ui, sizeof(*ui));

    ui->state = UI_STATE_HIDE;
    ui->area.x = 0;
    ui->area.y = 0;
    ui->area.w = w;
    ui->area.h = h;
    ui->childen = NULL;

    return 0;
}

void ui_destroy(struct ui *ui) {
    if (ui->childen) {
        /* first destroy childen recursively */
        while (sf_list_cnt(ui->childen)) {
            struct ui *child = *(struct ui **) sf_list_head(ui->childen);
            ui_remove_child(ui, child);
            ui_destroy(child);
        }

        sf_list_destroy(ui->childen);
        sf_free(ui->childen);
        ui->childen = NULL;
    }

    if (ui->on_destroy) {
        ui->on_destroy(ui);
    }
}

int ui_add_child(struct ui *ui, struct ui *child, int x, int y) {
    if (ui->childen == NULL) {
        sf_list_def_t def;

        sf_memzero(&def, sizeof(def));
        def.size = sizeof(struct ui *);

        ui->childen = sf_alloc(sizeof(*ui->childen));
        sf_list_init(ui->childen, &def);
    }

    child->area.x = x;
    child->area.y = y;
    child->parent = ui;
    sf_list_push(ui->childen, &child);
    return 0;
}

void ui_remove_child(struct ui *ui, struct ui *child) {
    sf_list_iter_t iter;

    if (ui->childen && sf_list_begin(ui->childen, &iter)) do {
        struct ui *p = *(struct ui **) sf_list_iter_elt(&iter);
        if (child == p) {
            if (child->parent != ui) {
                sf_log(SF_LOG_WARN, "ui_remove_child: parent's child doesn't"
                       " belevie it is his parent.");
            }
            sf_list_remove(ui->childen, &iter);
            child->parent = NULL;
            return;
        }
    } while (sf_list_iter_next(&iter));

    sf_log(SF_LOG_WARN, "ui_remove_child: don't have any childen.");
}

void ui_show(struct ui *ui) {
    sf_list_iter_t iter;

    if (ui->childen && sf_list_begin(ui->childen, &iter)) do {
        ui_show(*(struct ui **) sf_list_iter_elt(&iter));
    } while (sf_list_iter_next(&iter));

    if (ui->state != UI_STATE_SHOW && ui->on_show) {
        ui->on_show(ui);
    }
    ui->state = UI_STATE_SHOW;
}

void ui_hide(struct ui *ui) {
    sf_list_iter_t iter;

    if (ui->childen && sf_list_begin(ui->childen, &iter)) do {
        ui_hide(*(struct ui **) sf_list_iter_elt(&iter));
    } while (sf_list_iter_next(&iter));

    if (ui->state != UI_STATE_HIDE && ui->on_hide) {
        ui->on_hide(ui);
    }
    ui->state = UI_STATE_HIDE;
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


int ui_manager_init(struct ui_manager *uim) {
    uim->ui_pressed = NULL;
    uim->root = NULL;
    return 0;
}

void ui_manager_destroy(struct ui_manager *uim) {
}


/**
 * @return whether the event will pass to parent ?
 *         0     - no
 *         other - yes
 */
static int handle_press_event(struct ui_manager *uim, int action,
                              struct ui *ui, int x, int y) {
    sf_list_iter_t  iter;
    int             ispass = 1;

    if (ui->childen && sf_list_rbegin(ui->childen, &iter)) do {
        struct ui *ui = *(struct ui**) sf_list_iter_elt(&iter);

        if (ui->state != UI_STATE_SHOW
            || !sf_rect_iscontain(&ui->area, x, y)) {
            continue;
        }

        ispass = handle_press_event(uim, action, ui,
                                    x - ui->area.x, y - ui->area.y);

        if (!ispass) {
            break;
        }
    } while (sf_list_iter_next(&iter));

    if (ispass && (uim->ui_pressed != NULL ? uim->ui_pressed == ui : 1)) {
        switch (action) {
        case KEY_PRESS:
            if (ui->on_press && uim->ui_pressed == NULL) {
                ispass = ui->on_press(ui, x, y);
            }
            break;
        case KEY_LONG_PRESS:
            if (ui->on_long_press) {
                ispass = ui->on_long_press(ui, x, y);
            }
            break;
        case KEY_TAP:
            if (ui->on_tap) {
                ispass = ui->on_tap(ui, x, y);
            }
            break;
        }

        /* for now, just notify the top ui for the press event */
        ispass = 0;
        uim->ui_pressed = ui;
    }

    return ispass;
}

static void update_ui(struct ui *ui, struct input_manager *im, double dt) {
    sf_list_iter_t iter;

    if (ui->childen && sf_list_begin(ui->childen, &iter)) do {
        update_ui(*(struct ui**) sf_list_iter_elt(&iter), im, dt);
    } while (sf_list_iter_next(&iter));

    if (ui->state == UI_STATE_SHOW && ui->on_update) {
        ui->on_update(ui, im, dt);
    }
}

void ui_manager_update(struct ui_manager *uim, struct input_manager *im,
                       double dt) {
    if (uim->root && uim->root->state == UI_STATE_SHOW
        && im->keys[KEY_MB_LEFT] != KEY_RELEASE) {
        int x, y;
        x = im->mouse.x - uim->root->area.x;
        y = im->mouse.y - uim->root->area.y;
        handle_press_event(uim, im->keys[KEY_MB_LEFT], uim->root, x, y);
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

static void render_ui(struct ui *ui)
{
    int             x, y;
    sf_list_iter_t  iter;

    if (ui->state == UI_STATE_HIDE) {
        return;
    }

    renderer2d_get_viewport(&x, &y, NULL, NULL);
    x += ui->area.x;
    y += ui->area.y;
    renderer2d_push_viewport(x, y, ui->area.w, ui->area.h);
    if (ui->on_render) {
        ui->on_render(ui);
    }

    if (ui->childen && sf_list_begin(ui->childen, &iter)) do {
        render_ui(*(struct ui **) sf_list_iter_elt(&iter));
    } while (sf_list_iter_next(&iter));

    renderer2d_pop_viewport();
}

void ui_manager_render(struct ui_manager *uim)
{
    if (uim->root && uim->root->state != UI_STATE_HIDE) {
        render_ui(uim->root);
    }
}

void ui_manager_set_root(struct ui_manager *uim, struct ui *root) {
    uim->root = root;
}
