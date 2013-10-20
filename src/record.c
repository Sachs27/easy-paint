#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sf_debug.h>

#include "record.h"
#include "canvas.h"


#define RECORD_PIXEL_NALLOC 1024

static int record_undo_1(struct record *record, struct canvas *canvas) {
    struct sf_array        *segment;
    struct record_pixel    *pixel;

    if (record->canvas || !record_canundo(record)) {
        return -1;
    }

    if (record->nrecords) {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);
    } else {
        --record->nsegments;
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);
        record->nrecords = segment->nelts;
    }

    pixel = SF_ARRAY_NTH(segment, record->nrecords - 1);

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
            segment = *(struct sf_array **)
                       SF_ARRAY_NTH(record->segments, record->nsegments - 1);
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

    segment = *(struct sf_array **)
               SF_ARRAY_NTH(record->segments, record->nsegments - 1);

    if (record->nrecords >= segment->nelts) {
        ++record->nsegments;
        record->nrecords = 1;
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);
    } else {
        ++record->nrecords;
    }

    pixel = SF_ARRAY_NTH(segment, record->nrecords - 1);

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

    record = malloc(sizeof(*record));

    if (record_init(record) != 0) {
        free(record);
        return NULL;
    }

    return record;
}

int record_init(struct record *record) {
    record->version = RECORD_VERSION;
    record->nsegments = 0;
    record->nrecords = 0;
    if ((record->segments = sf_array_create(sizeof(struct sf_array *),
                                            SF_ARRAY_NALLOC)) == NULL) {
        return -1;
    }
    record->canvas = NULL;
    return 0;
}

int record_load(struct record *record, const char *pathname) {
    static struct canvas *canvas = NULL;
    FILE       *f;
    uint32_t    nsegments, nrecords, i, j;

    if (canvas == NULL) {
        canvas = canvas_create(1, 1);
    }

    if (record->canvas != NULL) {
        return -1;
    }

    f = fopen(pathname, "rb");

    if (f == NULL) {
        return -1;
    }

    fread(&record->version, sizeof(record->version), 1, f);

    if (record->version != RECORD_VERSION) {
        dprintf("Failed to load record, unsupport version: %d\n", record->version);
        fclose(f);
        return -1;
    }

    record->nsegments = 0;
    record->nrecords = 0;
    canvas_clear(canvas);

    fread(&nsegments, sizeof(nsegments), 1, f);

    for (i = 0; i < nsegments; ++i) {
        record_begin(record, canvas);

        fread(&nrecords, sizeof(nrecords), 1, f);

        for (j = 0; j < nrecords; ++j) {
            struct record_pixel pixel;

            fread(&pixel.x, sizeof(pixel.x), 1, f);
            fread(&pixel.y, sizeof(pixel.y), 1, f);
            fread(pixel.ncolor, sizeof(pixel.ncolor), 1, f);
            canvas_plot(canvas, pixel.x, pixel.y,
                        pixel.ncolor[0], pixel.ncolor[1],
                        pixel.ncolor[2], pixel.ncolor[3]);
        }
        record_end(record);
    }

    fclose(f);

    record->nsegments = 0;
    record->nrecords = 0;

    return 0;
}

void record_save(struct record *record, const char *pathname) {
    struct sf_array *segment;
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
        segment = *(struct sf_array **) SF_ARRAY_NTH(record->segments, i);
        assert(segment->nelts > 0);
        fwrite(&segment->nelts, sizeof(segment->nelts), 1, f);

        SF_ARRAY_BEGIN(segment, struct record_pixel, pixel);
            fwrite(&pixel->x, sizeof(pixel->x), 1, f);
            fwrite(&pixel->y, sizeof(pixel->y), 1, f);
            fwrite(pixel->ncolor, sizeof(pixel->ncolor), 1, f);
        SF_ARRAY_END();
    }

    if (record->nrecords) {
        struct record_pixel *pixel;
        int n;

        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);
        assert(segment->nelts > 0);
        n = record->nrecords;
        fwrite(&n, sizeof(n), 1, f);
        for (i = 0; i < n; ++i) {
            pixel = SF_ARRAY_NTH(segment, i);
            fwrite(&pixel->x, sizeof(pixel->x), 1, f);
            fwrite(&pixel->y, sizeof(pixel->y), 1, f);
            fwrite(pixel->ncolor, sizeof(pixel->ncolor), 1, f);
        }
    }

    fclose(f);
}

void record_begin(struct record *record, struct canvas *canvas) {
    struct sf_array *segment;
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
    if (record->nsegments >= record->segments->nelts) {
        segment = sf_array_create(sizeof(struct record_pixel),
                                  RECORD_PIXEL_NALLOC);
        sf_array_push(record->segments, &segment);
    } else {
        int i;
        /*
         * Make sure (nsegments - 1) is the last segment,
         * and (nsegments - 1) is empty.
         */
        for (i = record->nsegments - 1; i < record->segments->nelts; ++i) {
            sf_array_clear(*(struct sf_array **)
                            SF_ARRAY_NTH(record->segments, i), NULL);
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
    struct sf_array    *segment;
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

    segment = *(struct sf_array **)
               SF_ARRAY_NTH(record->segments, record->nsegments - 1);
    if (record->nrecords < segment->nelts) {
        struct record_pixel *p = SF_ARRAY_NTH(segment, record->nrecords);
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
    struct sf_array    *segment;
    int                 n;

    if (record->canvas || !record_canundo(record)) {
        return;
    }

    if (record->nrecords) {
        n = record->nrecords;
    } else {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 2);
        n = segment->nelts;
    }

    record_undo_n(record, canvas, n);
}

void record_undo_n(struct record *record, struct canvas *canvas, uint32_t n) {
    while (n-- && record_undo_1(record, canvas) == 0) ;
}

int record_canredo(struct record *record) {
    struct sf_array *segment;
    int i;

    if (record->nsegments) {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);

        if (record->nrecords < segment->nelts) {
            return 1;
        }
    }

    for (i = record->nsegments; i < record->segments->nelts; ++i) {
        segment = *(struct sf_array **) SF_ARRAY_NTH(record->segments, i);
        if (segment->nelts > 0) {
            return 1;
        }
    }

    return 0;
}

void record_redo(struct record *record, struct canvas *canvas) {
    struct sf_array    *segment;
    int                 n;

    if (record->canvas || !record_canredo(record)) {
        return;
    }

    if (record->nsegments) {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments - 1);

        if (record->nrecords >= segment->nelts) {
            segment = *(struct sf_array **)
                       SF_ARRAY_NTH(record->segments, record->nsegments);
            n = segment->nelts;
        } else {
            n = segment->nelts - record->nrecords;
        }
    } else {
        segment = *(struct sf_array **)
                   SF_ARRAY_NTH(record->segments, record->nsegments);
        n = segment->nelts;
    }

    record_redo_n(record, canvas, n);
}

void record_redo_n(struct record *record, struct canvas *canvas, uint32_t n) {
    while (n-- && record_redo_1(record, canvas) == 0) ;
}

void record_adjust(struct record *record, int xmargin, int ymargin) {
    int xmin = INT_MAX, ymin = INT_MAX, xmax = 0, ymax = 0;
    int dx, dy;

    SF_ARRAY_BEGIN(record->segments, struct sf_array *, p);
        SF_ARRAY_BEGIN(*p, struct record_pixel, pixel);
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
        SF_ARRAY_END();
    SF_ARRAY_END();

    dx = xmargin - xmin;
    dy = ymargin - ymin;

    SF_ARRAY_BEGIN(record->segments, struct sf_array *, p);
        SF_ARRAY_BEGIN(*p, struct record_pixel, pixel);
            pixel->x += dx;
            pixel->y += dy;
        SF_ARRAY_END();
    SF_ARRAY_END();
}
