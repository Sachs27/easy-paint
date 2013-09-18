#ifndef APP_H
#define APP_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "sf.h"
#include "window.h"
#include "input_manager.h"
#include "canvas.h"


struct app {
    struct window *window;
    struct input_manager *im;

    struct canvas *canvas;


    void (*init_cb)(void);
    void (*update_cb)(double dt);
    void (*render_cb)(void);
};

extern struct app g_app;


#endif /* APP_H */
