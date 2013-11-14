#include <sf/utils.h>

#include "ui_replay_panel.h"
#include "system.h"
#include "resource_manager.h"


#define TOOLBOX_HEIGHT 48
#define TOOLBOX_MIN_WIDTH 288

#define REPLAY_SPEED_MIN     128
#define REPLAY_SPEED_MAX     5120
#define REPLAY_SPEED_DEFAULT 512
#define REPLAY_SPPED_DELTA   128


static void ui_replay_panel_reset(struct ui_replay_panel *urp) {
    urp->isreplay = 0;
    ui_imagebox_set_image(&urp->replay, urp->play_image);

    urp->isstop = 1;
    while (record_canredo(urp->record)) {
        record_redo(urp->record, &urp->canvas);
    }
}

static void replay_on_press(struct ui_imagebox *replay,
                            int n, int x[n], int y[n]) {
    struct ui_replay_panel *urp =
        sf_container_of(replay, struct ui_replay_panel, replay);

    if (urp->isstop) {
        urp->isstop = 0;
        while (record_canundo(urp->record)) {
            record_undo(urp->record, &urp->canvas);
        }
    }

    if (urp->isreplay) {
        urp->isreplay = 0;
        ui_imagebox_set_image(replay, urp->play_image);
    } else {
        urp->isreplay = 1;
        ui_imagebox_set_image(replay, urp->pause_image);
    }
}

static void stop_on_press(struct ui_imagebox *stop,
                          int n, int x[n], int y[n]) {
    struct ui_replay_panel *urp =
        sf_container_of(stop, struct ui_replay_panel, stop);
    ui_replay_panel_reset(urp);
}

static void rewind_on_press(struct ui_imagebox *rewind,
                            int n, int x[n], int y[n]) {
    struct ui_replay_panel *urp =
        sf_container_of(rewind, struct ui_replay_panel, rewind);
#if 0
    urp->replay_speed -= REPLAY_SPPED_DELTA;
    if (urp->replay_speed < REPLAY_SPEED_MIN) {
        urp->replay_speed = REPLAY_SPEED_MIN;
    }
#endif
    urp->record_id -= 1;
    if (urp->record_id < 0) {
        urp->record_id += RESOURCE_NRECORDS;
    }
    urp->record = resource_manager_get(urp->rm, RESOURCE_RECORD,
                                       urp->record_id);
    canvas_clear(&urp->canvas);
    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);
    ui_replay_panel_reset(urp);
}

static void fastforward_on_press(struct ui_imagebox *fastforward,
                                 int n, int x[n], int y[n]) {
    struct ui_replay_panel *urp =
        sf_container_of(fastforward, struct ui_replay_panel, fastforward);
#if 0
    urp->replay_speed += REPLAY_SPPED_DELTA;
    if (urp->replay_speed > REPLAY_SPEED_MAX) {
        urp->replay_speed = REPLAY_SPEED_MAX;
    }
#endif
    urp->record_id = (urp->record_id + 1) % RESOURCE_NRECORDS;
    urp->record = resource_manager_get(urp->rm, RESOURCE_RECORD,
                                       urp->record_id);
    canvas_clear(&urp->canvas);
    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);
    ui_replay_panel_reset(urp);
}

static void canvas_on_press(struct canvas *canvas,
                            int n, int x[n], int y[n]) {
    /* Just empty */
}

static void canvas_on_update(struct canvas *canvas, struct input_manager *im,
                             double dt) {
    static double ntoreplay = 0;

    struct ui_replay_panel *urp =
        sf_container_of(canvas, struct ui_replay_panel, canvas);

    if (urp->isreplay) {
        if (record_canredo(urp->record)) {
            int n;
            ntoreplay += urp->replay_speed * dt;
            n = ntoreplay;
            if (n > 0) {
                record_redo_n(urp->record, &urp->canvas, n);
                ntoreplay -= n;
            }
        } else {
            ui_replay_panel_reset(urp);
        }
    }
}

static void ui_replay_panel_on_resize(struct ui_replay_panel *urp,
                                      int w, int h) {
    ui_resize((struct ui *) &urp->canvas, w, h);
    ui_resize((struct ui *) &urp->toolbox, w, urp->toolbox.ui.area.h);
    ui_move((struct ui *) &urp->toolbox, 0,
            urp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    canvas_clear(&urp->canvas);
    record_adjust(urp->record, 0, 0, w, h);
    record_reset(urp->record);
    ui_replay_panel_reset(urp);
}


struct ui_replay_panel *ui_replay_panel_create(int w, int h,
                                               struct resource_manager *rm) {
    struct ui_replay_panel *urp;

    urp = sf_alloc(sizeof(*urp));
    ui_replay_panel_init(urp, w, h, rm);
    return urp;
}

int ui_replay_panel_init(struct ui_replay_panel *urp, int w, int h,
                         struct resource_manager *rm) {
    ui_init((struct ui *) urp, w, h);

    urp->rm = rm;

    urp->isreplay = 0;
    urp->isstop = 1;
    urp->replay_speed = REPLAY_SPEED_DEFAULT;

    canvas_init(&urp->canvas, w, h);
    UI_CALLBACK(&urp->canvas, update, canvas_on_update);
    UI_CALLBACK(&urp->canvas, press, canvas_on_press);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->canvas, 0, 0);

    urp->record_id = 0;
    urp->record = resource_manager_get(urp->rm, RESOURCE_RECORD,
                                       urp->record_id);

    urp->stop_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                           RESOURCE_TEXTURE_ICON_STOP);
    ui_imagebox_init(&urp->stop, 0, 0, urp->stop_image);
    UI_CALLBACK(&urp->stop, press, stop_on_press);

    urp->pause_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                            RESOURCE_TEXTURE_ICON_PAUSE);
    urp->play_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                           RESOURCE_TEXTURE_ICON_PLAY);
    ui_imagebox_init(&urp->replay, 0, 0, urp->play_image);
    UI_CALLBACK(&urp->replay, press, replay_on_press);

    urp->fastforward_image = resource_manager_get(
        rm, RESOURCE_TEXTURE, RESOURCE_TEXTURE_ICON_FASTFORWARD);
    ui_imagebox_init(&urp->fastforward, 0, 0, urp->fastforward_image);
    UI_CALLBACK(&urp->fastforward, press, fastforward_on_press);

    urp->rewind_image = resource_manager_get(rm, RESOURCE_TEXTURE,
                                             RESOURCE_TEXTURE_ICON_REWIND);
    ui_imagebox_init(&urp->rewind, 0, 0, urp->rewind_image);
    UI_CALLBACK(&urp->rewind, press, rewind_on_press);

    ui_toolbox_init(&urp->toolbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 240);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->replay);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->stop);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->rewind);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->fastforward);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->toolbox,
                 0, urp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    UI_CALLBACK(urp, resize, ui_replay_panel_on_resize);

    return 0;
}

