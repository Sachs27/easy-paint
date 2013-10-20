#ifndef APP_H
#define APP_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct window;
struct renderer2d;
struct input_manager;
struct ui_manager;
struct user_paint_panel;
struct ui_menu;
struct ui_replay_panel;

enum app_stage {
    APP_STAGE_DOODLE = 1,
    APP_STAGE_TEACHING,
    APP_STAGE_SETTING,
};

struct app {
    int                         stage;

    struct window              *window;
    struct renderer2d          *renderer2d;
    struct input_manager       *im;
    struct ui_manager          *uim;

    struct ui_imagebox         *logo;
    struct ui_imagebox         *label1;
    struct ui_imagebox         *label2;
    struct ui_imagebox         *label3;
    struct ui_menu             *menu;
    struct ui_imagebox         *menuicon;

    struct user_paint_panel    *upp;
    struct ui_replay_panel     *urp;
};

extern struct app g_app;


#endif /* APP_H */
