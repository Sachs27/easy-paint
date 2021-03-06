#ifndef RECORD_H
#define RECORD_H


#include <sf/list.h>

struct brush;
struct canvas;
#define RECORD_VERSION 100

struct record {
    int             version;
    sf_list_t       records;  /* elt: (struct record_point) */
    uint32_t        nrecords; /* nrecords <= sf_list_cnt(records) */

    int             play_pos; /* play_pos <= nrecords */
    struct brush   *brush;
};

int record_init(struct record *record);

void record_destroy(struct record *record);

void record_begin_plot(struct record *record, struct brush *brush);

void record_drawline(struct record *record, float x0, float y0,
                     float x1, float y1);

void record_end_plot(struct record *record);

void record_reset(struct record *record);

int record_replay(struct record *record, struct canvas *canvas, int step);

int record_canundo(struct record *record);

void record_undo(struct record *record, struct canvas *canvas);

int record_canredo(struct record *record);

void record_redo(struct record *record, struct canvas *canvas);


#endif /* RECORD_H */
