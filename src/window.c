#include <stdlib.h>
#include <string.h>

#include <sf_list.h>
#include "window.h"


static struct sf_list *windows = NULL;

static void on_window_size(GLFWwindow *handle, int w, int h) {
    SF_LIST_BEGIN(windows, struct window *, pwin);
        struct window *win = *pwin;
        if (win->handle == handle) {
            win->w = w;
            win->h = h;
            break;
        }
    SF_LIST_END();
}

struct window *window_create(const char *title, int w, int h) {
    struct window *win;

    if (windows == NULL) {
        windows = sf_list_create(sizeof(struct window *));
        assert(windows != NULL);
    }

    win = malloc(sizeof(*win));
    win->title = strdup(title);
    win->w = w;
    win->h = h;

    win->handle = glfwCreateWindow(w, h, title, NULL, NULL);

    if (win->handle == NULL) {
        free(win);
        return NULL;
    }

    sf_list_push(windows, &win);

    glfwSetWindowSizeCallback(win->handle, on_window_size);
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
