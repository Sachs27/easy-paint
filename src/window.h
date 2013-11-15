#ifndef WINDOW_H
#define WINDOW_H

#ifndef ANDROID
# include <GLFW/glfw3.h>
  struct window;
  typedef GLFWwindow window_handle_t;
  typedef void (window_on_resize_t)(struct window *, int, int);
#endif /* ANDROID */

struct window {
    const char   *title;
    int     w;
    int     h;

#ifndef ANDROID
    window_on_resize_t *on_resize;

    window_handle_t *handle;
#endif
};

struct window *window_create(const char *title, int w, int h);

struct window *window_get_instance(void);

void window_destroy(void);

int window_isopen(void);

#ifndef ANDROID
void window_on_resize(window_on_resize_t *resize_cb);
#endif


#endif
