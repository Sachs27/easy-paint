#ifndef WINDOW_H
#define WINDOW_H


#include <GLFW/glfw3.h>


typedef GLFWwindow window_handle_t;

struct window {
    char   *title;
    int     w;
    int     h;

    window_handle_t *handle;
};

struct window *window_create(const char *title, int w, int h);
void window_destroy(struct window *win);
int window_isopen(struct window *win);


#endif
