#ifndef APP_H
#define APP_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct window;
struct renderer2d;
struct input_manager;
struct ui_manager;
struct user_paint_panel;

struct app {
    struct window              *window;
    struct renderer2d          *renderer2d;
    struct input_manager       *im;
    struct ui_manager          *uim;

    struct user_paint_panel    *upp;
};

extern struct app g_app;


#endif /* APP_H */
