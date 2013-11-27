#include <assert.h>

#include <sf/utils.h>

#include "record.h"
#include "canvas.h"
#include "brush.h"


typedef enum {
    RECORD_BEGIN_PLOT,
    RECORD_KEYPOINT,
    RECORD_END_PLOT,
    RECORD_SET_BRUSH,
} record_point_type_t;

typedef struct {
    record_point_type_t type;
    union {
        struct {
            int x;
            int y;
        } keypoint;

        struct {
            struct brush brush;
        } set_brush;
    };
} record_point_t;


static void record_point_destroy(void *elt) {
}

static void record_point_do(record_point_t *rp, struct record *record,
                            struct canvas *canvas) {
    switch (rp->type) {
    case RECORD_BEGIN_PLOT:
        canvas_begin_plot(canvas);
        break;
    case RECORD_KEYPOINT:
        if (record->lastx < 0) {
            record->lastx = rp->keypoint.x;
            record->lasty = rp->keypoint.y;
        } else {
            brush_drawline(record->brush, canvas, record->lastx,
                           record->lasty, rp->keypoint.x, rp->keypoint.y);
            record->lastx = rp->keypoint.x;
            record->lasty = rp->keypoint.y;
        }
        break;
    case RECORD_END_PLOT:
        canvas_end_plot(canvas);
        record->lastx = -1;
        record->lasty = -1;
        break;
    case RECORD_SET_BRUSH:
        record->brush = &rp->set_brush.brush;
        break;
    }
}

static void record_save(struct record *record, record_point_t *rp) {
    if (record->nrecords == sf_list_cnt(&record->records)) {
        sf_list_push(&record->records, rp);
    } else {
        record_point_t *ptr = sf_list_nth(&record->records, record->nrecords);
        *ptr = *rp;
    }
    ++record->nrecords;
}


int record_init(struct record *record) {
    sf_list_def_t def;

    record->version = RECORD_VERSION;
    record->brush = NULL;
    record->nrecords = 0;
    record->play_pos = 0;
    record->lastx = -1;
    record->lasty = -1;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(record_point_t);
    def.free = record_point_destroy;
    return sf_list_init(&record->records, &def);
}

void record_destroy(struct record *record) {
    sf_list_destroy(&record->records);
}

void record_begin_plot(struct record *record) {
    record_point_t rp;

    rp.type = RECORD_BEGIN_PLOT;
    record_save(record, &rp);
}

void record_plot(struct record *record, float x, float y) {
    record_point_t rp;

    rp.type = RECORD_KEYPOINT;
    rp.keypoint.x = x;
    rp.keypoint.y = y;
    record_save(record, &rp);
}

void record_end_plot(struct record *record) {
    record_point_t rp;

    rp.type = RECORD_END_PLOT;
    record_save(record, &rp);
}

void record_set_brush(struct record *record, struct brush *brush) {
    record_point_t rp;

    rp.type = RECORD_SET_BRUSH;
    rp.set_brush.brush = *brush;
    record_save(record, &rp);
}

void record_playback(struct record *record, struct canvas *canvas) {
    sf_list_iter_t  iter;
    record_point_t *rp;
    int i = record->play_pos;

    if (record->nrecords <= 0 || i >= record->nrecords) {
        return;
    }

    sf_list_begin(&record->records, &iter);

    while (i-- && sf_list_iter_next(&iter)) /* void */;

    assert(i == -1);

    do {
        rp = sf_list_iter_elt(&iter);
        record_point_do(rp, record, canvas);
        ++record->play_pos;
    } while (sf_list_iter_next(&iter));
}

void record_undo(struct record *record, struct canvas *canvas) {
    int i;
    sf_list_iter_t iter;
    record_point_t *rp;

    if (record->nrecords == 0) {
        return;
    }

    sf_list_rbegin(&record->records, &iter);
    i = sf_list_cnt(&record->records);
    while (i > record->nrecords) {
        sf_list_iter_next(&iter);
        --i;
    }

    do {
        rp = sf_list_iter_elt(&iter);
        --i;
    } while (rp->type != RECORD_BEGIN_PLOT && sf_list_iter_next(&iter));

    if (rp->type != RECORD_BEGIN_PLOT) {
        return;
    }

    record->nrecords = i;

    if (record->nrecords) {
        i = 0;
        record->lastx = -1;
        record->lasty = -1;
        sf_list_begin(&record->records, &iter);

        do {
            rp = sf_list_iter_elt(&iter);
            record_point_do(rp, record, canvas);
            sf_list_iter_next(&iter);
        } while (i++ < record->nrecords);
    }
}
