#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include "window.h"


enum key_state {
    KEY_RELEASE,
    KEY_PRESS,
};

enum key {
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_H,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_ESC,
    NKEY,
};

struct mouse_button {
    enum key_state state;
    int x;
    int y;
};

struct input_manager {
    struct {
        int x, y;
        struct mouse_button mb1, mb2, mb3;
    } mouse;

    enum key_state keys[NKEY];

    struct window *win;
};


struct input_manager *input_manager_create(struct window *win);


#endif /* INPUT_MANAGER_H */
