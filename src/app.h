#ifndef APP_H
#define APP_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "sf.h"
#include "window.h"
#include "input_manager.h"
#include "ui.h"
#include "canvas.h"
#include "renderer2d.h"


struct app {
    struct window          *window;
    struct renderer2d      *renderer2d;
    struct input_manager   *im;
    struct ui_manager      *uim;
    struct canvas          *canvas;
    struct brush           *cur_brush;


    void (*init_cb)(void);
    void (*update_cb)(double dt);
    void (*render_cb)(void);
};

extern struct app g_app;


#endif /* APP_H */
