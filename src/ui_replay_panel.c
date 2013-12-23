#include <sf/utils.h>

#include "app.h"
#include "ui_replay_panel.h"
#include "system.h"
#include "resmgr.h"

#define REPLAY_STEP_MAX 8


static void ui_replay_panel_reset(struct ui_replay_panel *urp)
{
    if (urp->record) {
        if (urp->nreplayed) {
            record_reset(urp->record);
            urp->nreplayed = record_replay(urp->record, &urp->canvas,
                                           urp->nreplayed);
        } else {
            canvas_clear(&urp->canvas);
            record_undo(urp->record, &urp->canvas);
            record_redo(urp->record, &urp->canvas);
        }
    }
}

static int replay_on_tap(struct ui *ui, int x, int y)
{
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

static int stop_on_tap(struct ui *ui, int x, int y)
{
    struct ui_imagebox *stop = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(stop, struct ui_replay_panel, stop);

    ui_replay_panel_stop(urp);

    return 0;
}

static int rewind_on_tap(struct ui *ui, int x, int y)
{
    struct ui_imagebox *rewind = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(rewind, struct ui_replay_panel, rewind);

    --urp->step;
    if (urp->step < 1) {
        urp->step = 1;
    }
#if 0
    urp->replay_speed -= REPLAY_SPPED_DELTA;
    if (urp->replay_speed < REPLAY_SPEED_MIN) {
        urp->replay_speed = REPLAY_SPEED_MIN;
    }

    urp->record_id -= 1;

    /* skip 0 */
    if (urp->record_id <= 0) {
        urp->record_id = sf_array_cnt(&urp->records) - 1;
    }

    if (urp->record_id) {
        urp->record = *(struct record **)
                       sf_array_nth(&urp->records, urp->record_id);
    } else {
        urp->record = NULL;
    }

    canvas_clear(&urp->canvas);

    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);

    urp->nreplayed = 0;
    urp->isadjust = 1;
    ui_replay_panel_stop(urp);
    ui_replay_panel_reset(urp);
#endif
    return 0;
}

static int fastforward_on_tap(struct ui *ui, int x, int y)
{
    struct ui_imagebox *fastforward = (struct ui_imagebox *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(fastforward, struct ui_replay_panel, fastforward);

    ++urp->step;
    if (urp->step > REPLAY_STEP_MAX) {
        urp->step = REPLAY_STEP_MAX;
    }
#if 0
    urp->replay_speed += REPLAY_SPPED_DELTA;
    if (urp->replay_speed > REPLAY_SPEED_MAX) {
        urp->replay_speed = REPLAY_SPEED_MAX;
    }

    urp->record_id = (urp->record_id + 1) % sf_array_cnt(&urp->records);
    /* skip 0 */
    if (urp->record_id == 0) {
        urp->record_id = (urp->record_id + 1) % sf_array_cnt(&urp->records);
    }
    if (urp->record_id) {
        urp->record = *(struct record **)
                       sf_array_nth(&urp->records, urp->record_id);
    } else {
        urp->record = NULL;
    }
    canvas_clear(&urp->canvas);

    record_adjust(urp->record, 0, 0,
                  urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    record_reset(urp->record);

    urp->nreplayed = 0;
    urp->isadjust = 1;
    ui_replay_panel_stop(urp);
    ui_replay_panel_reset(urp);
#endif
    return 0;
}

static int canvas_on_tap(struct ui *ui, int x, int y)
{
    struct canvas *canvas = (struct canvas *) ui;
    struct ui_replay_panel *urp = sf_container_of(canvas,
                                                  struct ui_replay_panel,
                                                  canvas);

    if (urp->isreplay) {
        ui_replay_panel_pause(urp);
    } else {
        ui_replay_panel_play(urp);
    }

    return 0;
}

static void canvas_on_update(struct ui *ui, struct input_manager *im,
                             double dt)
{
    struct canvas *canvas = (struct canvas *) ui;
    struct ui_replay_panel *urp =
        sf_container_of(canvas, struct ui_replay_panel, canvas);

    if (urp->isreplay) {
        urp->dt += dt;
        if (urp->dt < 0.024) {
            return;
        }
        urp->dt -= 0.024;
        if (urp->nreplayed == 0){
            canvas_clear(&urp->canvas);
        }
        ++urp->nreplayed;
        if (record_replay(urp->record, &urp->canvas, urp->step) == 0) {
            ui_replay_panel_stop(urp);
        }
    }
}

static void ui_replay_panel_on_resize(struct ui *ui, int w, int h)
{
    struct ui_replay_panel *urp = (struct ui_replay_panel *) ui;

    ui_resize((struct ui *) &urp->canvas, w, h - TOOLBOX_HEIGHT);

    ui_resize((struct ui *) &urp->toolbox, w, TOOLBOX_HEIGHT);
    ui_move((struct ui *) &urp->toolbox, 0, urp->canvas.ui.area.h);

    ui_replay_panel_stop(urp);

    if (urp->record) {
        record_adjust(urp->record,
                      urp->canvas.ui.area.w, urp->canvas.ui.area.h);
    }
    ui_replay_panel_reset(urp);
}

static void ui_replay_panel_on_hide(struct ui *ui)
{
    struct ui_replay_panel *urp = (struct ui_replay_panel *) ui;

    urp->record = NULL;
}


int ui_replay_panel_init(struct ui_replay_panel *urp, int w, int h)
{
    ui_init((struct ui *) urp, w, h);

    urp->step = 1;
    urp->isreplay = 0;
    urp->isstop = 1;
    urp->dt = 0;

    canvas_init(&urp->canvas, w, h - TOOLBOX_HEIGHT);
    UI_CALLBACK(&urp->canvas, update, canvas_on_update);
    UI_CALLBACK(&urp->canvas, tap, canvas_on_tap);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->canvas, 0, 0);

    sf_array_def_t def;
    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct record *);
    def.nalloc = rm_get_user_define_record_count();

    urp->record = NULL;

    urp->stop_image = rm_load_texture(RES_TEXTURE_ICON_STOP);
    ui_imagebox_init(&urp->stop, 0, 0, urp->stop_image);
    UI_CALLBACK(&urp->stop, tap, stop_on_tap);

    urp->pause_image = rm_load_texture(RES_TEXTURE_ICON_PAUSE);
    urp->play_image = rm_load_texture(RES_TEXTURE_ICON_PLAY);
    ui_imagebox_init(&urp->replay, 0, 0, urp->play_image);
    UI_CALLBACK(&urp->replay, tap, replay_on_tap);

    urp->fastforward_image = rm_load_texture(RES_TEXTURE_ICON_FASTFORWARD);
    ui_imagebox_init(&urp->fastforward, 0, 0, urp->fastforward_image);
    UI_CALLBACK(&urp->fastforward, tap, fastforward_on_tap);

    urp->rewind_image = rm_load_texture(RES_TEXTURE_ICON_REWIND);
    ui_imagebox_init(&urp->rewind, 0, 0, urp->rewind_image);
    UI_CALLBACK(&urp->rewind, tap, rewind_on_tap);

    ui_toolbox_init(&urp->toolbox, w, TOOLBOX_HEIGHT, 128, 128, 128, 255);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->rewind);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->fastforward);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->replay);
    ui_toolbox_add_button(&urp->toolbox, (struct ui *) &urp->stop);
    ui_add_child((struct ui *) urp, (struct ui *) &urp->toolbox,
                 0, urp->canvas.ui.area.h - TOOLBOX_HEIGHT);

    UI_CALLBACK(urp, hide, ui_replay_panel_on_hide);
    UI_CALLBACK(urp, resize, ui_replay_panel_on_resize);

    return 0;
}

void ui_replay_panel_set_record(struct ui_replay_panel *urp, struct record *r)
{
    urp->record = r;
    record_reset(r);
    record_adjust(urp->record, urp->canvas.ui.area.w, urp->canvas.ui.area.h);
}

void ui_replay_panel_play(struct ui_replay_panel *urp)
{
    if (urp->record == NULL) {
        return;
    }

    if (urp->isstop) {
        urp->isstop = 0;
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

void ui_replay_panel_stop(struct ui_replay_panel *urp)
{
    ui_replay_panel_pause(urp);

    urp->isstop = 1;
    urp->nreplayed = 0;

    ui_replay_panel_reset(urp);
}
