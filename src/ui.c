#include <string.h>
#include <stdlib.h>

#include "ui.h"


int ui_init(struct ui *ui, int w, int h) {
    memset(ui, 0, sizeof(*ui));

    ui->state = UI_STATE_HIDE;
    ui->area.x = 0;
    ui->area.y = 0;
    ui->area.w = w;
    ui->area.h = h;
    ui->childs = sf_list_create(sizeof(struct ui *));

    return 0;
}

int ui_add_child(struct ui *ui, struct ui *child, int x, int y) {
    child->area.x = x;
    child->area.y = y;
    child->parent = ui;
    sf_list_push(ui->childs, &child);
    return 0;
}

void ui_show(struct ui *ui) {
    ui->state = UI_STATE_SHOW;
    SF_LIST_BEGIN(ui->childs, struct ui *, pui);
        ui_show(*pui);
    SF_LIST_END();
    if (ui->on_show) {
        ui->on_show(ui);
    }
}

void ui_hide(struct ui *ui) {
    SF_LIST_BEGIN(ui->childs, struct ui *, pui);
        ui_hide(*pui);
    SF_LIST_END();
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
    struct ui_manager *uim;

    uim = malloc(sizeof(*uim));
    uim->uis = sf_list_create(sizeof(struct ui *));
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
    int i;
    struct sf_array *ax, *ay;
    int ispass = 1;

    ax = sf_array_create(sizeof(int), SF_ARRAY_NALLOC);
    ay = sf_array_create(sizeof(int), SF_ARRAY_NALLOC);

    SF_LIST_BEGIN_R(ui->childs, struct ui *, pui);
        if ((*pui)->state != UI_STATE_SHOW) {
            continue;
        }
        sf_array_clear(ax, NULL);
        sf_array_clear(ay, NULL);
        
        for (i = 0; i < n; ++i) {
            if (sf_rect_iscontain(&(*pui)->area, x[i], y[i])) {
                int tmp;
                tmp = x[i] - (*pui)->area.x;
                sf_array_push(ax, &tmp);
                tmp = y[i] - (*pui)->area.y;
                sf_array_push(ay, &tmp);
            }
        }

        if (ax->nelts == 0) {
            continue;
        }

        ispass = handle_press_event(uim, *pui, ax->nelts, ax->elts, ay->elts);
        if (!ispass) {
            break;
        }
    SF_LIST_END();
    
    if (ispass && ui->on_press) {
        ispass = ui->on_press(ui, n, x, y);
        
        /* for now, just notify the top ui for the press event */
        ispass = 0;
        uim->ui_pressed = ui;
    }

    sf_array_destroy(ax, NULL);
    sf_array_destroy(ay, NULL);
    
    return ispass;
}

static void update_ui(struct ui *ui, struct input_manager *im, double dt) {
    SF_LIST_BEGIN_R(ui->childs, struct ui *, pui);
        update_ui(*pui, im, dt);
    SF_LIST_END();
    
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
    int x, y;

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
    SF_LIST_BEGIN(ui->childs, struct ui *, pui);
        render_ui(*pui, r);
    SF_LIST_END();
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
