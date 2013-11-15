#include <sf/utils.h>

#include "user_learn_panel.h"
#include "ui_replay_panel.h"
#include "user_paint_panel.h"


static void user_learn_panel_on_resize(struct ui *ui, int w, int h) {
    struct user_learn_panel *ulp = (struct user_learn_panel *) ui;
    ulp->urp_h = h / 2;
    ui_resize((struct ui *) &ulp->urp, w, ulp->urp_h);
    ui_resize((struct ui *) &ulp->upp, w, h - ulp->urp_h);
    ui_move((struct ui *) &ulp->upp, 0, h - ulp->urp_h);
}


int user_learn_panel_init(struct user_learn_panel *ulp, int w, int h,
                          struct resource_manager *rm) {
    ui_init((struct ui *) ulp, w, h);

    ulp->urp_h = h / 2;
    ui_replay_panel_init(&ulp->urp, w, ulp->urp_h, rm);
    ui_add_child((struct ui *) ulp, (struct ui *) &ulp->urp, 0, 0);

    user_paint_panel_init(&ulp->upp, w, h - ulp->urp_h, rm);
    ui_add_child((struct ui *) ulp, (struct ui *) &ulp->upp, 0, ulp->urp_h);

    UI_CALLBACK(ulp, resize, user_learn_panel_on_resize);

    return 0;
}
