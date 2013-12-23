#ifndef UI_REPLAY_PANEL_H
#define UI_REPLAY_PANEL_H


#include <sf/array.h>

#include "ui.h"
#include "canvas.h"
#include "record.h"
#include "ui_toolbox.h"
#include "ui_imagebox.h"

struct resource_manager;


struct ui_replay_panel {
    struct ui           ui;             /* inherit from ui, so this must be
                                         * the first element of the struct  */

    struct canvas       canvas;

    struct record      *record;

    int                 step;
    int                 nreplayed;
    int16_t             isreplay;
    int16_t             isstop;
    double              dt;

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


int ui_replay_panel_init(struct ui_replay_panel *urp, int w, int h);

void ui_replay_panel_set_record(struct ui_replay_panel *urp, struct record *r);

void ui_replay_panel_play(struct ui_replay_panel *urp);

void ui_replay_panel_pause(struct ui_replay_panel *urp);

void ui_replay_panel_stop(struct ui_replay_panel *urp);


#endif /* UI_REPLAY_PANEL_H */
