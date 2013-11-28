#include <assert.h>

#include <sf/utils.h>

#include "record.h"
#include "canvas.h"
#include "brush.h"


typedef enum {
    RECORD_BEGIN_PLOT,
    RECORD_DRAWLINE,
    RECORD_END_PLOT,
    RECORD_SET_BRUSH,
} record_point_type_t;

typedef struct {
    record_point_type_t type;
    union {
        struct {
            float x0, y0;
            float x1, y1;
        } drawline;
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
    case RECORD_DRAWLINE:
        brush_drawline(record->brush, canvas, rp->drawline.x0, rp->drawline.y0,
                       rp->drawline.x1, rp->drawline.y1);
        break;
    case RECORD_END_PLOT:
        canvas_end_plot(canvas);
        break;
    case RECORD_SET_BRUSH:
        record->brush = &rp->set_brush.brush;
        break;
    }
}

static record_point_t *record_save(struct record *record, record_point_t *rp) {
    record_point_t *ptr;
    if (record->nrecords == sf_list_cnt(&record->records)) {
        ptr = sf_list_push(&record->records, rp);
    } else {
        ptr = sf_list_nth(&record->records, record->nrecords);
        *ptr = *rp;
    }
    ++record->nrecords;
    return ptr;
}


int record_init(struct record *record) {
    sf_list_def_t def;

    record->version = RECORD_VERSION;
    record->brush = NULL;
    record->nrecords = 0;
    record->play_pos = 0;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(record_point_t);
    def.free = record_point_destroy;
    return sf_list_init(&record->records, &def);
}

void record_destroy(struct record *record) {
    sf_list_destroy(&record->records);
}

void record_begin_plot(struct record *record, struct brush *brush) {
    record_point_t rp;

    if (brush_cmp(record->brush, brush) != 0) {
        rp.type = RECORD_SET_BRUSH;
        rp.set_brush.brush = *brush;
        record->brush = &(((record_point_t *) record_save(record, &rp))
                         ->set_brush.brush);
    }

    rp.type = RECORD_BEGIN_PLOT;
    record_save(record, &rp);
}

void record_drawline(struct record *record, float x0, float y0,
                     float x1, float y1) {
    record_point_t rp;

    rp.type = RECORD_DRAWLINE;
    rp.drawline.x0 = x0;
    rp.drawline.y0 = y0;
    rp.drawline.x1 = x1;
    rp.drawline.y1 = y1;
    record_save(record, &rp);
}

void record_end_plot(struct record *record) {
    record_point_t rp;

    rp.type = RECORD_END_PLOT;
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

    rp = sf_list_head(&record->records);
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

    /* the records always have a RECORD_SET_BRUSH record at the begining */
    assert(i > 0);
    record->nrecords = i;

    if (record->nrecords) {
        sf_list_begin(&record->records, &iter);

        while (i--) {
            rp = sf_list_iter_elt(&iter);
            record_point_do(rp, record, canvas);
            sf_list_iter_next(&iter);
        }
    }
}

void record_redo(struct record *record, struct canvas *canvas) {
}
