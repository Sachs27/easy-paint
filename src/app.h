#ifndef APP_H
#define APP_H


#include "user_paint_panel.h"
#include "user_learn_panel.h"
#include "ui_select_record.h"
#include "ui_menu.h"


#define WINDOW_TITLE "Easy Paint"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

struct window;
struct renderer2d;
struct input_manager;

enum app_stage {
    APP_STAGE_DOODLE = 1,
    APP_STAGE_TEACHING,
    APP_STAGE_SELECT,
    APP_STAGE_SETTING,
};

struct app {
    sf_bool_t                   inited;
    int                         stage;

    struct window              *window;
    struct input_manager       *im;
    struct ui_manager           uim;


    struct ui                   root;

    struct ui_imagebox          logo;
    struct ui_imagebox          label1;
    struct ui_imagebox          label2;
    struct ui_imagebox          label3;
    struct ui_menu              menu;
    struct ui_imagebox          menuicon;

    struct user_paint_panel     upp;
    struct user_learn_panel     ulp;
    struct ui_select_record     sr;
};


int app_init(const char *res_path, const char *save_path);

void app_destory(void);

void app_on_update(double dt);

void app_on_render(void);

void app_on_resize(struct window *win, int w, int h);


extern struct app g_app;


#endif /* APP_H */
