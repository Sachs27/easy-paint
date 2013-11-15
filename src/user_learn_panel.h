#ifndef USER_LEARN_PANEL_H
#define USER_LEARN_PANEL_H


#include "ui.h"
#include "user_paint_panel.h"
#include "ui_replay_panel.h"

struct resource_manager;

struct user_learn_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */

    int                      urp_h;
    struct ui_replay_panel   urp;
    struct user_paint_panel  upp;
};


int user_learn_panel_init(struct user_learn_panel *ulp, int w, int h,
                          struct resource_manager *rm);


#endif /* USER_LEARN_PANEL_H */
