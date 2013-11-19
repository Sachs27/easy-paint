#include <stdlib.h>

#include <sf/utils.h>

#include "../../input_manager.h"


static struct input_manager *input_manager = NULL;

struct input_manager *input_manager_create(struct window *win) {
    if (input_manager) {
        free(input_manager);
        input_manager = NULL;
    }

    input_manager = sf_calloc(sizeof(*input_manager));
    input_manager->win = win;

    return input_manager;
}

void input_manager_destroy(void) {
    struct window *win;

    if (input_manager == NULL) {
        return;
    }

    win = input_manager->win;
}

void input_manager_touch_down(int n, int x[n], int y[n]) {
    input_manager->mouse.x = x[0];
    input_manager->mouse.y = y[0];
    input_manager->keys[KEY_MB_LEFT] = KEY_PRESS;
}

void input_manager_touch_up(void) {
    input_manager->keys[KEY_MB_LEFT] = KEY_RELEASE;
}

void input_manager_update(void) {
    struct input_manager *im = input_manager;
    int k;

    for (k = 0; k < NKEYS; ++k) {
        if (im->last_keys[k] == KEY_PRESS) {
            im->keys[k] = KEY_REPEAT;
        }
        im->last_keys[k] = im->keys[k];
    }
}
