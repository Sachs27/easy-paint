#include <assert.h>

#include <sf/utils.h>
#include <sf/log.h>

#include "record.h"
#include "canvas.h"
#include "brush.h"
#include "filesystem.h"
#include "renderer2d.h"


#define RECORD_SIG "EASYPAINTRECORD"
#define RECORD_SIG_LEN 16

enum record_point_type {
    RECORD_BEGIN_PLOT,
    RECORD_DRAWLINE,
    RECORD_END_PLOT,
    RECORD_SET_BRUSH,
};

struct record_point {
    int type;
    union {
        struct {
            float x0, y0;
            float x1, y1;
        } drawline;
        struct {
            struct brush brush;
        } set_brush;
    };
};

static void record_point_destroy(void *elt)
{
}

static void record_point_do(struct record_point *rp, struct record *record,
                            struct canvas *canvas)
{
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

static struct record_point *record_push(struct record *record,
                                        struct record_point *rp)
{
    struct record_point *ptr;
    if (record->nrecords == sf_list_cnt(&record->records)) {
        ptr = sf_list_push(&record->records, rp);
    } else {
        ptr = sf_list_nth(&record->records, record->nrecords);
        *ptr = *rp;
    }
    ++record->nrecords;
    return ptr;
}

static void clean_cache(struct record *record)
{
    while (record->nrecords < sf_list_cnt(&record->records)) {
        sf_list_pop(&record->records);
    }
}


int record_init(struct record *record, int w, int h)
{
    sf_list_def_t def;

    record->version = RECORD_VERSION;
    record->w = w;
    record->h = h;
    record->brush = NULL;
    record->nrecords = 0;
    record->play_pos = 0;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct record_point);
    def.free = record_point_destroy;
    return sf_list_init(&record->records, &def);
}

void record_destroy(struct record *record)
{
    sf_list_destroy(&record->records);
}

int record_copy(struct record *dst, struct record *src)
{
    sf_list_def_t def;
    int           i;

    dst->version  = src->version;
    dst->w        = src->w;
    dst->h        = src->h;
    dst->nrecords = src->nrecords;
    dst->play_pos = 0;
    dst->brush    = NULL;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct record_point);
    def.free = record_point_destroy;
    if (sf_list_init(&dst->records, &def) != SF_OK) {
        return SF_ERR;
    }

    for (i = 0; i < dst->nrecords; ++i) {
        sf_list_push(&dst->records, sf_list_nth(&src->records, i));
    }

    return SF_OK;
}

void record_begin_plot(struct record *record, struct brush *brush)
{
    struct record_point rp;

    clean_cache(record);

    if (brush_cmp(record->brush, brush) != 0) {
        rp.type = RECORD_SET_BRUSH;
        rp.set_brush.brush = *brush;
        record->brush = &(((struct record_point *) record_push(record, &rp))
                         ->set_brush.brush);
    }

    rp.type = RECORD_BEGIN_PLOT;
    record_push(record, &rp);
}

void record_drawline(struct record *record, float x0, float y0,
                     float x1, float y1)
{
    struct record_point rp;

    clean_cache(record);

    rp.type = RECORD_DRAWLINE;
    rp.drawline.x0 = x0;
    rp.drawline.y0 = y0;
    rp.drawline.x1 = x1;
    rp.drawline.y1 = y1;
    record_push(record, &rp);
}

void record_end_plot(struct record *record)
{
    struct record_point rp;

    clean_cache(record);

    rp.type = RECORD_END_PLOT;
    record_push(record, &rp);
}

int record_replay(struct record *record, struct canvas *canvas, int step)
{
    int nreplayed = 0;
    int i = record->play_pos;
    sf_list_iter_t  iter;
    struct record_point *rp;

    if (step == 0) {
        step = -1;
    }

    if (record->nrecords <= 0 || record->play_pos >= record->nrecords) {
        return 0;
    }

    i = 0;
    sf_list_begin(&record->records, &iter);

    while (i < record->play_pos && sf_list_iter_next(&iter)) {
        ++i;
    }

    if (i == 0) {
        canvas_clear(canvas);
    }

    do {
        rp = sf_list_iter_elt(&iter);
        if (rp->type == RECORD_DRAWLINE) {
            --step;
            ++nreplayed;
        }
        record_point_do(rp, record, canvas);
        ++record->play_pos;
    } while (step && record->play_pos < record->nrecords
             && sf_list_iter_next(&iter));

    return nreplayed;
}

int record_canundo(struct record *record)
{
    int i = 0;
    sf_list_iter_t iter;
    struct record_point *rp;

    if (record == NULL) {
        return 0;
    }

    if (record->nrecords == 0) {
        return 0;
    }

    sf_list_begin(&record->records, &iter);
    do {
        rp = sf_list_iter_elt(&iter);
        if (rp->type == RECORD_BEGIN_PLOT) {
            return 1;
        }
    } while (++i < record->nrecords && sf_list_iter_next(&iter));

    return 0;
}

void record_undo(struct record *record, struct canvas *canvas)
{
    int i;
    sf_list_iter_t iter;
    struct record_point *rp;

    if (record->nrecords == 0) {
        return;
    }

    sf_list_rbegin(&record->records, &iter);
    i = sf_list_cnt(&record->records);
    while (i > record->nrecords) {
        sf_list_iter_next(&iter);
        --i;
    }

    /* search for the last RECORD_BEGIN_PLOT */
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

    canvas_clear(canvas);

    sf_list_begin(&record->records, &iter);

    do {
        rp = sf_list_iter_elt(&iter);
        record_point_do(rp, record, canvas);
        sf_list_iter_next(&iter);
    } while (--i);
}

int record_canredo(struct record *record)
{
    int i;
    sf_list_iter_t iter;
    struct record_point *rp;

    if (record == NULL) {
        return 0;
    }

    i = sf_list_cnt(&record->records);
    if (i == 0 || i <= record->nrecords) {
        return 0;
    }

    sf_list_rbegin(&record->records, &iter);
    --i;
    do {
        rp = sf_list_iter_elt(&iter);
        if (rp->type == RECORD_END_PLOT) {
            return 1;
        }
    } while (--i >= record->nrecords && sf_list_iter_next(&iter));

    return 0;
}

void record_redo(struct record *record, struct canvas *canvas)
{
    uint32_t i;
    sf_list_iter_t iter;
    sf_list_iter_t tmp;
    struct record_point *rp;

    if (record->nrecords >= sf_list_cnt(&record->records)) {
        return;
    }

    sf_list_begin(&record->records, &iter);
    i = 0;
    while (i < record->nrecords) {
        ++i;
        sf_list_iter_next(&iter);
    }

    /* search for the next RECORD_END_PLOT */
    tmp = iter;
    do {
        rp = sf_list_iter_elt(&tmp);
        ++i;
    } while (rp->type != RECORD_END_PLOT && sf_list_iter_next(&tmp));

    if (rp->type != RECORD_END_PLOT) {
        return;
    }

    assert(i > record->nrecords);
    while (record->nrecords < i) {
        rp = sf_list_iter_elt(&iter);
        record_point_do(rp, record, canvas);
        sf_list_iter_next(&iter);
        ++record->nrecords;
    }
}

void record_reset(struct record *record)
{
    record->play_pos = 0;
}

/*
 * record file format:
 *
 * ---------------------
 * |   0|   1|   2|   3|   (32 bit)
 * ---------------------
 * |  E |  A |  S |  Y |
 * |  P |  A |  I |  N |
 * |  T |  R |  E |  C |
 * |  O |  R |  D | \0 |
 * |     version       |
 * |      width        |
 * |      height       |
 * |type|    ....      |
 */
int record_load(struct record *record, const char *filename)
{
    char sig[RECORD_SIG_LEN];
    uint8_t type;
    uint32_t version;
    int ret;
    int w, h;
    struct fs_file fin;
    struct brush brush;

    if ((ret = fs_file_open(&fin, filename, "rb")) != SF_OK) {
        return ret;
    }

    fs_file_read(&fin, sig, RECORD_SIG_LEN);

    if (strncmp(sig, RECORD_SIG, RECORD_SIG_LEN)) {
        sf_log(SF_LOG_ERR, "failed to load record %s: not a record",
               filename);
        fs_file_close(&fin);
        return SF_ERR;
    }

    fs_file_read(&fin, &version, sizeof(version));

    if (version != RECORD_VERSION) {
        sf_log(SF_LOG_ERR, "failed to load record %s: unknown version",
               filename);
        fs_file_close(&fin);
        return SF_ERR;
    }

    fs_file_read(&fin, &w, sizeof(w));
    fs_file_read(&fin, &h, sizeof(h));

    record_init(record, w, h);

    while (fs_file_read(&fin, &type, sizeof(type))) {
        float pos[4];

        switch (type) {
        case RECORD_BEGIN_PLOT:
            /* correct record file must have set brush before begining plot */
            record_begin_plot(record, &brush);
            break;
        case RECORD_DRAWLINE:
            fs_file_read(&fin, pos, sizeof(pos));
            record_drawline(record, pos[0], pos[1], pos[2], pos[3]);
            break;
        case RECORD_END_PLOT:
            record_end_plot(record);
            break;
        case RECORD_SET_BRUSH:
            fs_file_read(&fin, &brush, sizeof(brush));
            break;
        }
    }

    fs_file_close(&fin);
    return SF_OK;
}

int record_save(struct record *record, const char *filename)
{
    int ret;
    uint32_t version = RECORD_VERSION;
    sf_list_iter_t iter;
    uint32_t i = 0;
    struct fs_file fout;

    if ((ret = fs_file_open(&fout, filename, "wb+")) != SF_OK) {
        return ret;
    }

    /* sig */
    fs_file_write(&fout, RECORD_SIG, RECORD_SIG_LEN);

    /* version */
    fs_file_write(&fout, &version, sizeof(version));

    fs_file_write(&fout, &record->w, sizeof(record->w));
    fs_file_write(&fout, &record->h, sizeof(record->h));

    /* <type, data> pairs */
    if (sf_list_begin(&record->records, &iter)) do {
        uint8_t type;
        float pos[4];
        struct record_point *rp = sf_list_iter_elt(&iter);

        type = rp->type;
        fs_file_write(&fout, &type, sizeof(type));
        switch (type) {
        case RECORD_DRAWLINE:
            pos[0] = rp->drawline.x0;
            pos[1] = rp->drawline.y0;
            pos[2] = rp->drawline.x1;
            pos[3] = rp->drawline.y1;
            fs_file_write(&fout, pos, sizeof(pos));
            break;
        case RECORD_SET_BRUSH:
            fs_file_write(&fout, &rp->set_brush.brush, sizeof(struct brush));
            break;
        }
    } while (++i < record->nrecords && sf_list_iter_next(&iter));

    fs_file_close(&fout);

    return SF_OK;
}

int record_to_texture(struct record *record, struct texture *texture,
                      int w, int h)
{
    struct canvas canvas;
    /*int ow, oh;*/

    canvas_init(&canvas, w, h);

    texture_init_2d(texture, w, h);

    /*ow = record->w;*/
    /*oh = record->h;*/
    record_adjust(record, w, h);

    if (record_canundo(record)) {
        record_undo(record, &canvas);
        record_redo(record, &canvas);
        texture_copy(texture, &canvas.content);
    } else {
        texture_clear(texture, 0.0f, 0.0f, 0.0f, 0.0f);
    }

    /*record_adjust(record, ow, oh);*/

    canvas_destroy(&canvas);

    return SF_OK;
}

int record_adjust(struct record *record, int w, int h)
{
    sf_list_iter_t iter;
    float oradio = record->w * 1.0 / record->h;
    float nradio = w * 1.0 / h;
    float xradio;
    float yradio;
    int nw, nh;

    if (nradio == oradio) {
        if (w == record->w || h == record->h) {
            return SF_OK;
        }
        nw = w;
        nh = h;
    } else if (nradio > oradio) {
        if (h == record->h) {
            return SF_OK;
        }
        nh = h;
        nw = nh * oradio;
    } else /* nradio < oradio */ {
        if (w == record->h) {
            return SF_OK;
        }
        nw = w;
        nh = nw / oradio;
    }

    xradio = nw * 1.0 / record->w;
    yradio = nh * 1.0 / record->h;

    if (sf_list_begin(&record->records, &iter)) do {
        struct record_point *rp = sf_list_iter_elt(&iter);
        if (rp->type == RECORD_DRAWLINE) {
            rp->drawline.x0 = rp->drawline.x0 * xradio;
            rp->drawline.x1 = rp->drawline.x1 * xradio;

            rp->drawline.y0 = rp->drawline.y0 * yradio;
            rp->drawline.y1 = rp->drawline.y1 * yradio;
        } else if (rp->type == RECORD_SET_BRUSH) {
            float r = rp->set_brush.brush.radius;
            r =  r * (xradio + yradio) / 2;
            rp->set_brush.brush.radius = r;
        }
    } while (sf_list_iter_next(&iter));

    record->w = nw;
    record->h = nh;

    return SF_OK;
}
