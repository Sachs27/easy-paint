#include <string.h>
#include <stdlib.h>

#include "ui.h"


int ui_init(struct ui *ui, int w, int h) {
    memset(ui, 0, sizeof(*ui));

    ui->state = UI_STATE_SHOW;
    ui->area.x = 0;
    ui->area.y = 0;
    ui->area.w = w;
    ui->area.h = h;

    return 0;
}


/* ======================================================================= */


struct ui_manager *ui_manager_create(void) {
    struct ui_manager *uim;

    uim = malloc(sizeof(*uim));
    uim->uis = sf_list_create(sizeof(struct ui *));
    uim->ispressed = 0;
    uim->ui_pressed = NULL;

    return uim;
}

void ui_manager_update(struct ui_manager *uim, struct input_manager *im,
                       double dt) {
    if (!uim->ispressed && im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        SF_LIST_BEGIN_R(uim->uis, struct ui *, pui);
            /* retrieve the list in reverser order */
            struct ui *ui = *pui;

            if (ui->on_press && ui->state == UI_STATE_SHOW
                && sf_rect_iscontain(&ui->area, im->mouse.x, im->mouse.y)) {
                int x = im->mouse.x - ui->area.x;
                int y = im->mouse.y - ui->area.y;

                uim->ispressed = 1;
                uim->ui_pressed = ui;

                ui->on_press(ui, 1, &x,  &y);
                break;
            }
        SF_LIST_END();
    } else if (im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
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

        if (ui->state == UI_STATE_SHOW && ui->on_update) {
            ui->on_update(ui, im, dt);
        }
    SF_LIST_END();
}

void ui_manager_render(struct ui_manager *uim, struct renderer2d *r) {
    SF_LIST_BEGIN(uim->uis, struct ui *, pui);
        struct ui *ui = *pui;

        if (ui->on_render == NULL || ui->state == UI_STATE_HIDE) {
            continue;
        }

        renderer2d_push_viewport(r, ui->area.x, ui->area.y,
                                    ui->area.w, ui->area.h);
        ui->on_render(ui, r);

        renderer2d_pop_viewport(r);

    SF_LIST_END();
}

void ui_manager_push(struct ui_manager *uim, int x, int y, struct ui *ui) {
    ui->area.x = x;
    ui->area.y = y;
    sf_list_push(uim->uis, &ui);
    if (ui->on_push) {
        ui->on_push(ui, uim, x, y);
    }
}
