#ifndef UI_REPLAY_PANEL_H
#define UI_REPLAY_PANEL_H


#include "ui.h"
#include "canvas.h"
#include "record.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"


struct ui_replay_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */
    struct canvas       canvas;
    struct record       record;

    int                 isreplay;
    int                 isstop;
    int                 replay_speed;

    struct ui_toolbox   toolbox;

    struct texture     *stop_image;
    struct ui_imagebox  stop;

    struct texture     *pause_image;
    struct texture     *play_image;
    struct ui_imagebox  replay;

    struct texture     *fastforward_image;
    struct ui_imagebox  fastforward;

    struct texture     *rewind_image;
    struct ui_imagebox  rewind;
};


struct ui_replay_panel *ui_replay_panel_create(int w, int h);

int ui_replay_panel_init(struct ui_replay_panel *urp, int w, int h);


#endif /* UI_REPLAY_PANEL_H */
