#ifndef WINDOW_H
#define WINDOW_H

#if defined(__WIN32__) || defined(__linux__)
#include <GLFW/glfw3.h>

struct window;

typedef GLFWwindow window_handle_t;

typedef void (window_on_resize_t)(struct window *, int, int);
#endif /* defined(__WIN32__) || defined(__linux__) */

struct window {
    char   *title;
    int     w;
    int     h;

#if defined(__WIN32__) || defined(__linux__)
    window_on_resize_t *on_resize;

    window_handle_t *handle;
#endif /* defined(__WIN32__) || defined(__linux__) */
};

struct window *window_create(const char *title, int w, int h);

struct window *window_get_instance(void);

void window_destroy(void);

int window_isopen(void);

void window_on_resize(window_on_resize_t *resize_cb);


#endif
