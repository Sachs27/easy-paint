#include <sf/utils.h>

#include "ui_replay_panel.h"
#include "system.h"
#include "resmgr.h"


#define TOOLBOX_HEIGHT 48
#define TOOLBOX_MIN_WIDTH 288

#define REPLAY_SPEED_MIN     128
#define REPLAY_SPEED_MAX     5120
#define REPLAY_SPEED_DEFAULT 512
#define REPLAY_SPPED_DELTA   128


static void ui_replay_panel_reset(struct ui_replay_panel *urp) {
    if (urp->isreplay) {
        ui_replay_panel_pause(urp);
    }

    if (urp->isstop == 0) {
        urp->isstop = 1;
        if (urp->record) {
            record_reset(urp->record);
            while (record_replay(urp->record, &urp->canvas, 0)) /* void */;
        }
    }
}

static int replay_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *replay = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(replay, struct ui_replay_panel, replay);

    if (urp->isreplay) {
        ui_replay_panel_pause(urp);
    } else {
        ui_replay_panel_play(urp);
    }

    return 0;
}

static int stop_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *stop = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(stop, struct ui_replay_panel, stop);

    ui_replay_panel_reset(urp);

    return 0;
}

static int rewind_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *rewind = (struct ui_imagebox *) ui;
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
    urp->record = rm_load_record(NULL);
    canvas_clear(&urp->canvas);
#if 0
    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);
#endif
    ui_replay_panel_reset(urp);

    return 0;
}

static int fastforward_on_press(struct ui *ui, int x, int y) {
    struct ui_imagebox *fastforward = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(fastforward, struct ui_replay_panel, fastforward);
#if 0
    urp->replay_speed += REPLAY_SPPED_DELTA;
    if (urp->replay_speed > REPLAY_SPEED_MAX) {
        urp->replay_speed = REPLAY_SPEED_MAX;
    }
#endif
    urp->record_id = (urp->record_id + 1) % RESOURCE_NRECORDS;
    urp->record = rm_load_record(NULL);
    canvas_clear(&urp->canvas);
#if 0
    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);
#endif
    ui_replay_panel_reset(urp);
    return 0;
}

static int canvas_on_press(struct ui *ui, int x, int y) {
    /* Just empty */
    return 0;
}

static void canvas_on_update(struct ui *ui, struct input_manager *im,
                             double dt) {
    struct canvas *canvas = (struct canvas *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(canvas, struct ui_replay_panel, canvas);

    if (urp->isreplay) {
        urp->dt += dt;
        if (urp->dt < 0.024) {
            return;
        }
        urp->dt -= 0.024;
        if (record_replay(urp->record, &urp->canvas, 1) == 0) {
            ui_replay_panel_pause(urp);
        }
    }
}

static void ui_replay_panel_on_resize(struct ui *ui, int w, int h) {
    struct ui_replay_panel *urp = (struct ui_replay_panel *) ui;
    ui_resize((struct ui *) &urp->canvas, w, h);
    ui_resize((struct ui *) &urp->toolbox, w, urp->toolbox.ui.area.h);
    ui_move((struct ui *) &urp->toolbox, 0,
            urp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    canvas_clear(&urp->canvas);
#if 0
    record_adjust(urp->record, 0, 0, w, h);
#endif
    if (urp->record) {
        record_reset(urp->record);
    }

    ui_replay_panel_reset(urp);
}


int ui_replay_panel_init(struct ui_replay_panel *urp, int w, int h)
{
    ui_init((struct ui *) urp, w, h);

    urp->isreplay = 0;
    urp->isstop = 1;
    urp->dt = 0;

    canvas_init(&urp->canvas, w, h);
    UI_CALLBACK(&urp->canvas, update, canvas_on_update);
    UI_CALLBACK(&urp->canvas, press, canvas_on_press);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->canvas, 0, 0);

    urp->record_id = 0;
    urp->record = rm_load_record(NULL);

    urp->stop_image = rm_load_texture(RES_TEXTURE_ICON_STOP);
    ui_imagebox_init(&urp->stop, 0, 0, urp->stop_image);
    UI_CALLBACK(&urp->stop, press, stop_on_press);

    urp->pause_image = rm_load_texture(RES_TEXTURE_ICON_PAUSE);
    urp->play_image = rm_load_texture(RES_TEXTURE_ICON_PLAY);
    ui_imagebox_init(&urp->replay, 0, 0, urp->play_image);
    UI_CALLBACK(&urp->replay, press, replay_on_press);

    urp->fastforward_image = rm_load_texture(RES_TEXTURE_ICON_FASTFORWARD);
    ui_imagebox_init(&urp->fastforward, 0, 0, urp->fastforward_image);
    UI_CALLBACK(&urp->fastforward, press, fastforward_on_press);

    urp->rewind_image = rm_load_texture(RES_TEXTURE_ICON_REWIND);
    ui_imagebox_init(&urp->rewind, 0, 0, urp->rewind_image);
    UI_CALLBACK(&urp->rewind, press, rewind_on_press);

    ui_toolbox_init(&urp->toolbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 240);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->rewind);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->fastforward);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->replay);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->stop);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->toolbox,
                 0, urp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    UI_CALLBACK(urp, resize, ui_replay_panel_on_resize);

    return 0;
}

void ui_replay_panel_set_record(struct ui_replay_panel *urp, struct record *r)
{
    urp->record = r;
    record_reset(r);
}

void ui_replay_panel_play(struct ui_replay_panel *urp)
{
    if (urp->isstop) {
        urp->isstop = 0;
        canvas_clear(&urp->canvas);
        record_reset(urp->record);
    } else if (record_replay(urp->record, &urp->canvas, 1) == 0) {
        canvas_clear(&urp->canvas);
        record_reset(urp->record);
    }

    urp->isreplay = 1;
    ui_imagebox_set_image(&urp->replay, urp->pause_image);
}

void ui_replay_panel_pause(struct ui_replay_panel *urp)
{
    urp->isreplay = 0;
    ui_imagebox_set_image(&urp->replay, urp->play_image);
}
