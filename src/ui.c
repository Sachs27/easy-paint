#include <stdlib.h>

#include "app.h"
#include "ui.h"


extern struct app g_app;


void ui_init(struct ui *ui, int w, int h) {
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


struct ui_manager *ui_manager_create(void) {
    struct ui_manager *uim;

    uim = malloc(sizeof(*uim));
    uim->uis = sf_list_create(sizeof(struct ui *));
    uim->ispressed = 0;
    uim->ui_pressed = NULL;

    return uim;
}

void ui_manager_update(struct ui_manager *uim, double dt) {
    if (!uim->ispressed && g_app.im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        SF_LIST_BEGIN_R(uim->uis, struct ui *, pui);
            /* retrieve the list in reverser order */
            struct ui *ui = *pui;

            if (ui->on_press
                && sf_rect_iscontain(&ui->area, g_app.im->mouse.x,
                                                g_app.im->mouse.y)) {
                int x = g_app.im->mouse.x - ui->area.x;
                int y = g_app.im->mouse.y - ui->area.y;

                uim->ispressed = 1;
                uim->ui_pressed = ui;

                ui->on_press(ui, 1, &x,  &y);
                break;
            }
        SF_LIST_END();
    } else if (g_app.im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
        if (uim->ispressed) {
            uim->ispressed = 0;
            if (uim->ui_pressed->on_release) {
                uim->ui_pressed->on_release(uim->ui_pressed);
            }
            uim->ui_pressed = NULL;
        }
    }

    SF_LIST_BEGIN(uim->uis, struct ui *, pui);
        struct ui *ui = *pui;

        if (ui->on_update) {
            ui->on_update(ui, dt);
        }
    SF_LIST_END();
}

void ui_manager_render(struct ui_manager *uim) {
    SF_LIST_BEGIN(uim->uis, struct ui *, pui);
        struct ui *ui = *pui;

        if (ui->on_render == NULL) {
            continue;
        }

        renderer2d_push_viewport(g_app.renderer2d, ui->area.x, ui->area.y,
                                 ui->area.w, ui->area.h);
        ui->on_render(ui);

        renderer2d_pop_viewport(g_app.renderer2d);

    SF_LIST_END();
}

void ui_manager_push(struct ui_manager *uim, int x, int y, struct ui *ui) {
    ui->area.x = x;
    ui->area.y = y;
    sf_list_push(uim->uis, &ui);
}
