#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <sf/utils.h>
#include <sf/log.h>

#include "record.h"
#include "canvas.h"


#define RECORD_PIXEL_NALLOC 1024

static int record_undo_1(struct record *record, struct canvas *canvas) {
    sf_array_t             *segment;
    struct record_pixel    *pixel;

    if (record->canvas || !record_canundo(record)) {
        return -1;
    }

    if (record->nrecords) {
        segment = sf_array_nth(&record->segments, record->nsegments - 1);
    } else {
        --record->nsegments;
        segment = sf_array_nth(&record->segments, record->nsegments - 1);
        record->nrecords = segment->nelts;
    }

    pixel = sf_array_nth(segment, record->nrecords - 1);

    /*
     * Make sure that canvas __will_not__ record this plot.
     */
    assert(record->canvas == NULL);
    canvas_plot(canvas, pixel->x, pixel->y,
                pixel->ocolor[0], pixel->ocolor[1],
                pixel->ocolor[2], pixel->ocolor[3]);
    --record->nrecords;

    if (record->nrecords == 0) {
        --record->nsegments;
        if (record->nsegments > 0) {
            segment = sf_array_nth(&record->segments, record->nsegments - 1);
            record->nrecords = segment->nelts;
        }
    }

    return 0;
}

static int record_redo_1(struct record *record, struct canvas *canvas) {
    struct sf_array        *segment;
    struct record_pixel    *pixel;

    if (record->canvas || !record_canredo(record)) {
        return -1;
    }

    if (record->nsegments == 0) {
        ++record->nsegments;
    }

    segment = sf_array_nth(&record->segments, record->nsegments - 1);

    if (record->nrecords >= segment->nelts) {
        ++record->nsegments;
        record->nrecords = 1;
        segment = sf_array_nth(&record->segments, record->nsegments - 1);
    } else {
        ++record->nrecords;
    }

    pixel = sf_array_nth(segment, record->nrecords - 1);

    /*
     * Make sure that canvas __will_not__ record this plot.
     */
    assert(record->canvas == NULL);
    canvas_plot(canvas, pixel->x, pixel->y,
                pixel->ncolor[0], pixel->ncolor[1],
                pixel->ncolor[2], pixel->ncolor[3]);

    return 0;
}


struct record *record_create(void) {
    struct record *record;

    record = sf_alloc(sizeof(*record));

    if (record_init(record) != 0) {
        sf_free(record);
        return NULL;
    }

    return record;
}

int record_init(struct record *record) {
    sf_array_def_t def;

    record->version = RECORD_VERSION;
    record->nsegments = 0;
    record->nrecords = 0;

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(sf_array_t);
    if (sf_array_init(&record->segments, &def) != SF_OK) {
        return -1;
    }
    record->canvas = NULL;
    return 0;
}

int record_load_zip(struct record *record, struct zip *archive,
                    const char *filename) {
    static struct canvas   *canvas = NULL;
    FILE                   *raw_file;
    struct zip_file        *zip_file;
    uint32_t    nsegments, nrecords, i, j;

    if (canvas == NULL) {
        canvas = canvas_create(1, 1);
    }

    record_init(record);

    if (archive) {
        zip_file = zip_fopen(archive, filename, 0);
        if (!zip_file) {
            return -1;
        }
    } else {
        raw_file = fopen(filename, "rb");
        if (raw_file == NULL) {
            return -1;
        }
    }
#define READ_BYTE(buf, n) do {          \
    if (archive) {                      \
        zip_fread(zip_file, buf, n);    \
    } else {                            \
        fread(buf, 1, n, raw_file);     \
    }                                   \
} while (0)

#define CLOSE() do {                \
    if (archive) {                  \
        zip_fclose(zip_file);       \
    } else {                        \
        fclose(raw_file);           \
    }                               \
} while (0)

    READ_BYTE(&record->version, sizeof(record->version));

    if (record->version != RECORD_VERSION) {
        sf_log(SF_LOG_ERR, "Failed to load record, unsupport version: %d\n",
               record->version);
        CLOSE();
        return -1;
    }

    record->nsegments = 0;
    record->nrecords = 0;
    canvas_clear(canvas);

    READ_BYTE(&nsegments, sizeof(nsegments));

    for (i = 0; i < nsegments; ++i) {
        record_begin(record, canvas);

        READ_BYTE(&nrecords, sizeof(nrecords));

        for (j = 0; j < nrecords; ++j) {
            struct record_pixel pixel;

            READ_BYTE(&pixel.x, sizeof(pixel.x));
            READ_BYTE(&pixel.y, sizeof(pixel.y));
            READ_BYTE(pixel.ncolor, sizeof(pixel.ncolor));
            canvas_plot(canvas, pixel.x, pixel.y,
                        pixel.ncolor[0], pixel.ncolor[1],
                        pixel.ncolor[2], pixel.ncolor[3]);
        }
        record_end(record);
    }

    CLOSE();

    record->nsegments = 0;
    record->nrecords = 0;

    return 0;
}

int record_load(struct record *record, const char *pathname) {
    return record_load_zip(record, NULL, pathname);
}

void record_save(struct record *record, const char *pathname) {
    sf_array_t *segment;
    uint32_t i, nsegments;
    FILE *f;

    if (record->nsegments == 0) {
        return;
    }

    f = fopen(pathname, "wb+");

    if (f == NULL) {
        return;
    }

    fwrite(&record->version, sizeof(record->version), 1, f);

    if (record->nrecords) {
        nsegments = record->nsegments;
    } else {
        nsegments = record->nsegments - 1;
    }

    fwrite(&nsegments, sizeof(nsegments), 1, f);

    for (i = 0; i < record->nsegments - 1; ++i) {
        sf_array_iter_t iter;

        segment = sf_array_nth(&record->segments, i);
        assert(segment->nelts > 0);
        fwrite(&segment->nelts, sizeof(segment->nelts), 1, f);

        if (sf_array_begin(&record->segments, &iter)) do {
            struct record_pixel *pixel = sf_array_iter_elt(&iter);
            fwrite(&pixel->x, sizeof(pixel->x), 1, f);
            fwrite(&pixel->y, sizeof(pixel->y), 1, f);
            fwrite(pixel->ncolor, sizeof(pixel->ncolor), 1, f);
        } while (sf_array_iter_next(&iter));
    }

    if (record->nrecords) {
        int n;

        segment = sf_array_nth(&record->segments, record->nsegments - 1);
        assert(sf_array_cnt(segment) > 0);
        n = record->nrecords;
        fwrite(&n, sizeof(n), 1, f);
        for (i = 0; i < n; ++i) {
            struct record_pixel *pixel;

            pixel = sf_array_nth(segment, i);
            fwrite(&pixel->x, sizeof(pixel->x), 1, f);
            fwrite(&pixel->y, sizeof(pixel->y), 1, f);
            fwrite(pixel->ncolor, sizeof(pixel->ncolor), 1, f);
        }
    }

    fclose(f);
}

void record_reset(struct record *record) {
    record->nsegments = 0;
    record->nrecords = 0;
}

void record_begin(struct record *record, struct canvas *canvas) {
    if (record->canvas) {
        return;
    }

    record->canvas = canvas;
    canvas->record = record;

    if (record->nsegments && record->nrecords == 0) {
        return;
    }

    ++record->nsegments;
    record->nrecords = 0;
    if (record->nsegments >= sf_array_cnt(&record->segments)) {
        sf_array_t     newsegment;
        sf_array_def_t def;

        sf_memzero(&def, sizeof(def));
        def.size = sizeof(struct record_pixel);
        sf_array_init(&newsegment, &def);
        sf_array_push(&record->segments, &newsegment);
    } else {
        int i;
        /*
         * Make sure (nsegments - 1) is the last segment,
         * and (nsegments - 1) is empty.
         */
        for (i = record->nsegments - 1;
             i < sf_array_cnt(&record->segments); ++i) {
            sf_array_clear(sf_array_nth(&record->segments, i));
        }
    }
}

void record_end(struct record *record) {
    record->canvas->record = NULL;
    record->canvas = NULL;
}

void record_record(struct record *record, int x, int y,
                   uint8_t or, uint8_t og, uint8_t ob, uint8_t oa,
                   uint8_t nr, uint8_t ng, uint8_t nb, uint8_t na) {
    sf_array_t         *segment;
    struct record_pixel pixel;

    pixel.x = x;
    pixel.y = y;
    pixel.ocolor[0] = or;
    pixel.ocolor[1] = og;
    pixel.ocolor[2] = ob;
    pixel.ocolor[3] = oa;
    pixel.ncolor[0] = nr;
    pixel.ncolor[1] = ng;
    pixel.ncolor[2] = nb;
    pixel.ncolor[3] = na;

    segment = sf_array_nth(&record->segments, record->nsegments - 1);
    if (record->nrecords < segment->nelts) {
        struct record_pixel *p = sf_array_nth(segment, record->nrecords);
        memcpy(p, &pixel, sizeof(pixel));
    } else {
        sf_array_push(segment, &pixel);
    }
    ++record->nrecords;
}

int record_canundo(struct record *record) {
    if (record->nrecords) {
        return 1;
    } else if (record->nsegments > 1) {
        return 1;
    }

    return 0;
}

void record_undo(struct record *record, struct canvas *canvas) {
    sf_array_t *segment;
    int         n;

    if (record->canvas || !record_canundo(record)) {
        return;
    }

    if (record->nrecords) {
        n = record->nrecords;
    } else {
        segment = sf_array_nth(&record->segments, record->nsegments - 2);
        n = sf_array_cnt(segment);
    }

    record_undo_n(record, canvas, n);
}

void record_undo_n(struct record *record, struct canvas *canvas, uint32_t n) {
    while (n-- && record_undo_1(record, canvas) == 0) ;
}

int record_canredo(struct record *record) {
    sf_array_t *segment;
    int         i;

    if (record->nsegments) {
        segment = sf_array_nth(&record->segments, record->nsegments - 1);

        if (record->nrecords < sf_array_cnt(segment)) {
            return 1;
        }
    }

    for (i = record->nsegments; i < sf_array_cnt(&record->segments); ++i) {
        segment = sf_array_nth(&record->segments, i);
        if (sf_array_cnt(segment) > 0) {
            return 1;
        }
    }

    return 0;
}

void record_redo(struct record *record, struct canvas *canvas) {
    sf_array_t *segment;
    int         n;

    if (record->canvas || !record_canredo(record)) {
        return;
    }

    if (record->nsegments) {
        segment = sf_array_nth(&record->segments, record->nsegments - 1);

        if (record->nrecords >= segment->nelts) {
            segment = sf_array_nth(&record->segments, record->nsegments);
            n = sf_array_cnt(segment);
        } else {
            n = sf_array_cnt(segment) - record->nrecords;
        }
    } else {
        segment = sf_array_nth(&record->segments, record->nsegments);
        n = segment->nelts;
    }

    record_redo_n(record, canvas, n);
}

void record_redo_n(struct record *record, struct canvas *canvas, uint32_t n) {
    while (n-- && record_redo_1(record, canvas) == 0) ;
}

void record_adjust(struct record *record, int x, int y, int w, int h) {
    int xmin = INT_MAX, ymin = INT_MAX, xmax = 0, ymax = 0;
    int dx, dy;
    sf_array_iter_t seg_iter, rp_iter;

    if (sf_array_begin(&record->segments, &seg_iter)) do {
        sf_array_t *p = sf_array_iter_elt(&seg_iter);

        if (sf_array_begin(p, &rp_iter)) do {
            struct record_pixel *pixel = sf_array_iter_elt(&rp_iter);
            if (pixel->x < xmin) {
                xmin = pixel->x;
            }
            if (pixel->x > xmax) {
                xmax = pixel->x;
            }
            if (pixel->y < ymin) {
                ymin = pixel->y;
            }
            if (pixel->y > ymax) {
                ymax = pixel->y;
            }
        } while (sf_array_iter_next(&rp_iter));
    } while (sf_array_iter_next(&seg_iter));

    dx = ((xmin - x) + (x + w - xmax)) / 2 - xmin;
    dy = ((ymin - y) + (y + h - ymax)) / 2 - ymin;

    if (sf_array_begin(&record->segments, &seg_iter)) do {
        sf_array_t *p = sf_array_iter_elt(&seg_iter);

        if (sf_array_begin(p, &rp_iter)) do {
            struct record_pixel *pixel = sf_array_iter_elt(&rp_iter);

            pixel->x += dx;
            pixel->y += dy;
        } while (sf_array_iter_next(&rp_iter));
    } while (sf_array_iter_next(&seg_iter));
}
