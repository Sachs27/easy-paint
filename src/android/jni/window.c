#include <stdlib.h>
#include "../../window.h"


static struct window *window = NULL;


struct window *window_create(const char *title, int w, int h) {
    if (window) {
        window_destroy();
    }

    window = malloc(sizeof(*window));
    window->title = strdup(title);
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

    free(window->title);
    free(window);
    window = NULL;
}