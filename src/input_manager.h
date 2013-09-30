#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include "window.h"


enum key_state {
    KEY_RELEASE,
    KEY_PRESS,
    KEY_REPEAT,
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
    NKEYS,
};

struct mouse_button {
    enum key_state state;
    int x;
    int y;
};

struct input_manager {
    struct {
        int x, y;

        enum key_state last_mbs[3];
        union {
            struct {
                 struct mouse_button mb1;
                 struct mouse_button mb2;
                 struct mouse_button mb3;
            };

            struct mouse_button mbs[3];
        };
    } mouse;

    enum key_state keys[NKEYS];
    enum key_state last_keys[NKEYS];

    struct window *win;
};


struct input_manager *input_manager_create(struct window *win);
void input_manager_update(struct input_manager *im);


#endif /* INPUT_MANAGER_H */
