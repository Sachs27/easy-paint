#ifndef WINDOW_H
#define WINDOW_H


#include <GLFW/glfw3.h>


struct window;

typedef GLFWwindow window_handle_t;

typedef void (window_on_resize_t)(struct window *, int, int);

struct window {
    char   *title;
    int     w;
    int     h;

    window_on_resize_t *on_resize;

    window_handle_t *handle;
};

struct window *window_create(const char *title, int w, int h);

void window_destroy(struct window *win);

int window_isopen(struct window *win);

inline static void window_on_resize(struct window *win,
                                    window_on_resize_t *resize_cb) {
    win->on_resize = resize_cb;
}


#endif
