#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include <sf/list.h>

#include "vector.h"
#include "window.h"


#define IM_KEY_LONG_PRESS_TIME 0.4
#define IM_KEY_MOVE_DELTA_PIXEL 2

enum key_state {
    KEY_RELEASE,
    KEY_PRESS,
    KEY_REPEAT,
    KEY_TAP,
    KEY_LONG_PRESS,
};

enum key {
    KEY_MB_LEFT,
#if defined(__WIN32__) || defined(__linux__)
    KEY_MB_MIDDLE,
    KEY_MB_RIGHT,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_H,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_ESC,
#endif /* defined(__WIN32__) || defined(__linux__) */
    NKEYS,
};


struct input_manager {
    sf_list_t mb_left_buffer;   /* elt: struct ivec2 */
    float     mb_left_time;
    int       is_mb_move;

    struct ivec2 mouse;

    enum key_state keys[NKEYS];

    struct window *win;
};

extern struct input_manager *input_manager;

struct input_manager *input_manager_create(struct window *win);

void input_manager_destroy(void);

void input_manager_update(double dt);

void input_manager_touch_down(int x, int y);

void input_manager_touch_up(void);


#endif /* INPUT_MANAGER_H */
