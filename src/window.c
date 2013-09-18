#include <stdlib.h>
#include <string.h>

#include "window.h"


struct window *window_create(const char *title, int w, int h) {
    struct window *win;

    win = malloc(sizeof(*win));
    win->title = strdup(title);
    win->w = w;
    win->h = h;

    win->handle = glfwCreateWindow(w, h, title, NULL, NULL);

    if (win->handle == NULL) {
        free(win);
        return NULL;
    }

    glfwMakeContextCurrent(win->handle);

    return win;
}

void window_destroy(struct window *win) {
    glfwDestroyWindow(win->handle);
    free(win->title);
    free(win);
}

int window_isopen(struct window *win) {
    return !glfwWindowShouldClose(win->handle);
}
