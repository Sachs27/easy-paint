#include <stdlib.h>

#include "ui.h"


void ui_init(struct ui *ui, int w, int h) {
    ui->state = UI_STATE_SHOW;
    ui->area.x = 0;
    ui->area.y = 0;
    ui->area.w = w;
    ui->area.h = h;

    ui->on_update = NULL;
    ui->on_render = NULL;
    ui->on_press = NULL;
    ui->on_release = NULL;
}


/* ======================================================================= */

struct ui_manager *ui_manager_create(struct input_manager *im,
                                     struct renderer2d *r) {
    struct ui_manager *uim;

    uim = malloc(sizeof(*uim));
    uim->im = im;
    uim->renderer2d = r;
    uim->uis = sf_list_create(sizeof(struct ui *));
    uim->ispressed = 0;
    uim->ui_pressed = NULL;

    return uim;
}

void ui_manager_update(struct ui_manager *uim, double dt) {
    if (!uim->ispressed && uim->im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        SF_LIST_BEGIN_R(uim->uis, struct ui *, pui);
            /* retrieve the list in reverser order */
            struct ui *ui = *pui;

            if (ui->on_press && ui->state == UI_STATE_SHOW
                && sf_rect_iscontain(&ui->area, uim->im->mouse.x,
                                                uim->im->mouse.y)) {
                int x = uim->im->mouse.x - ui->area.x;
                int y = uim->im->mouse.y - ui->area.y;

                uim->ispressed = 1;
                uim->ui_pressed = ui;

                ui->on_press(ui, 1, &x,  &y);
                break;
            }
        SF_LIST_END();
    } else if (uim->im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
        if (uim->ispressed) {
            if (uim->ui_pressed->on_release) {
                uim->ui_pressed->on_release(uim->ui_pressed);
            }
            uim->ui_pressed = NULL;
            uim->ispressed = 0;
        }
    }

    SF_LIST_BEGIN(uim->uis, struct ui *, pui);
        struct ui *ui = *pui;

        if (ui->on_update) {
            ui->on_update(ui, uim->im, dt);
        }
    SF_LIST_END();
}

void ui_manager_render(struct ui_manager *uim) {
    SF_LIST_BEGIN(uim->uis, struct ui *, pui);
        struct ui *ui = *pui;

        if (ui->on_render == NULL || ui->state == UI_STATE_HIDE) {
            continue;
        }

        renderer2d_push_viewport(uim->renderer2d, ui->area.x, ui->area.y,
                                 ui->area.w, ui->area.h);
        ui->on_render(ui, uim->renderer2d);

        renderer2d_pop_viewport(uim->renderer2d);

    SF_LIST_END();
}

void ui_manager_push(struct ui_manager *uim, int x, int y, struct ui *ui) {
    ui->area.x = x;
    ui->area.y = y;
    sf_list_push(uim->uis, &ui);
}
