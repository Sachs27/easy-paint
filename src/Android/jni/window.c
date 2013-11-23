#include <stdlib.h>
#include <sf/utils.h>
#include "../../window.h"


static struct window *window = NULL;


struct window *window_create(const char *title, int w, int h) {
    if (window) {
        window_destroy();
    }

    window = sf_alloc(sizeof(*window));
    window->title = title;
    window->w = w;
    window->h = h;

    return window;
}

struct window *window_get_instance(void) {
    return window;
}

void window_destroy(void) {
    if (window == NULL) {
        return;
    }

    free(window);
    window = NULL;
}
