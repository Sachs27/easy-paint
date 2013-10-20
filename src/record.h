#ifndef RECORD_H
#define RECORD_H


#include <sf_array.h>

struct canvas;

#define RECORD_VERSION 100

struct record_pixel {
    int     x, y;
    uint8_t ncolor[4];
    uint8_t ocolor[4];
};

struct record {
    int                 version;

    uint32_t            nsegments;

    /*
     * nrecords > 0 only when nsegments > 0 meaning that current
     * segment has records.
     *
     * nrecords == 0 means that current segment has no records.
     */
    uint32_t            nrecords;

    struct sf_array    *segments;       /* elt: (struct sf_array *)
                                         * |- elt: (struct record_pixel) */

    struct canvas      *canvas;
};


struct record *record_create(void);

int record_init(struct record *record);

int record_load(struct record *record, const char *pathname);

void record_save(struct record *record, const char *pathname);

void record_begin(struct record *record, struct canvas *canvas);

void record_end(struct record *record);

void record_record(struct record *record, int x, int y,
                   uint8_t or, uint8_t og, uint8_t ob, uint8_t oa,
                   uint8_t nr, uint8_t ng, uint8_t nb, uint8_t na);

int record_canundo(struct record *record);

void record_undo(struct record *record, struct canvas *canvas);

void record_undo_n(struct record *record, struct canvas *canvas, uint32_t n);

int record_canredo(struct record *record);

void record_redo(struct record *record, struct canvas *canvas);

void record_redo_n(struct record *record, struct canvas *canvas, uint32_t n);

void record_adjust(struct record *record, int xmargin, int ymargin);


#endif /* RECORD_H */
