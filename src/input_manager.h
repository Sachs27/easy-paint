#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include <sf/list.h>

#include "window.h"


enum key_state {
    KEY_RELEASE,
    KEY_PRESS,
    KEY_REPEAT,
};

struct im_mouse_position {
    int x;
    int y;
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
    sf_list_t mb_left_buffer;   /* elt: struct im_mouse_position */

    struct im_mouse_position mouse;

    enum key_state keys[NKEYS];

    struct window *win;
};


struct input_manager *input_manager_create(struct window *win);

void input_manager_destroy(void);

void input_manager_update();

#ifdef ANDROID
void input_manager_touch_down(int x, int y);
void input_manager_touch_up(void);
#endif /* ANDROID */

#endif /* INPUT_MANAGER_H */
