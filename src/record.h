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

    int             play_pos;
    struct brush   *brush;
    float           lastx, lasty;
};

int record_init(struct record *record);

void record_destroy(struct record *record);

void record_begin_plot(struct record *record);

void record_plot(struct record *record, float x, float y);

void record_end_plot(struct record *record);

void record_set_brush(struct record *record, struct brush *brush);

void record_playback(struct record *record, struct canvas *canvas);

void record_undo(struct record *record, struct canvas *canvas);


#endif /* RECORD_H */
