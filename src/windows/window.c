#include <string.h>

#include <sf/utils.h>

#include "../window.h"

static struct window *window = NULL;

static void on_window_size(GLFWwindow *handle, int w, int h) {
    if (w == 0 || h == 0) {
        return;
    }

    window->w = w;
    window->h = h;
    if (window->on_resize) {
        window->on_resize(window, w, h);
    }
}

struct window *window_create(const char *title, int w, int h) {
    if (window) {
        window_destroy();
    }

    window = sf_alloc(sizeof(*window));
    window->title = strdup(title);
    window->w = w;
    window->h = h;
    window->handle = glfwCreateWindow(w, h, title, NULL, NULL);

    if (window->handle == NULL) {
        sf_free(window);
        window = NULL;
        return NULL;
    }

    glfwSetWindowSizeCallback(window->handle, on_window_size);
    glfwMakeContextCurrent(window->handle);

    return window;
}

struct window *window_get_instance(void) {
    return window;
}

void window_destroy(void) {
    if (window == NULL) {
        return;
    }

    glfwDestroyWindow(window->handle);
    sf_free(window->title);
    sf_free(window);
    window = NULL;
}

int window_isopen(void) {
    if (window == NULL) {
        return 0;
    }

    return !glfwWindowShouldClose(window->handle);
}

void window_on_resize(window_on_resize_t *resize_cb) {
    if (window == NULL) {
        return;
    }
    window->on_resize = resize_cb;
}
