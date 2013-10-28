#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include "window.h"


enum key_state {
    KEY_RELEASE,
    KEY_PRESS,
    KEY_REPEAT,
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
    struct {
        int x, y;
    } mouse;

    enum key_state keys[NKEYS];
    enum key_state last_keys[NKEYS];

    struct window *win;
};


struct input_manager *input_manager_create(struct window *win);
void input_manager_update();

#ifdef ANDROID
void input_manager_touch_down(int n, int x[n], int y[n]);
void input_manager_touch_up(void);
#endif /* ANDROID */

#endif /* INPUT_MANAGER_H */
