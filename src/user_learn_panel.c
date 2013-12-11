#include <sf/utils.h>

#include "user_learn_panel.h"
#include "ui_replay_panel.h"
#include "user_paint_panel.h"


static void user_learn_panel_on_resize(struct ui *ui, int w, int h)
{
    struct user_learn_panel *ulp = (struct user_learn_panel *) ui;

    ui_resize((struct ui *) &ulp->urp, w, h / 2);
    ui_resize((struct ui *) &ulp->upp, w, h - h / 2);
    ui_move((struct ui *) &ulp->upp, 0, h / 2);
}


int user_learn_panel_init(struct user_learn_panel *ulp, int w, int h)
{
    ui_init((struct ui *) ulp, w, h);

    ui_replay_panel_init(&ulp->urp, w, h / 2);
    ui_add_child((struct ui *) ulp, (struct ui *) &ulp->urp, 0, 0);

    user_paint_panel_init(&ulp->upp, w, h - h / 2);
    ui_add_child((struct ui *) ulp, (struct ui *) &ulp->upp, 0, h / 2);

    UI_CALLBACK(ulp, resize, user_learn_panel_on_resize);

    return 0;
}
