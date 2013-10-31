#ifndef APP_H
#define APP_H

#define WINDOW_TITLE "Easy Paint"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 800

struct window;
struct renderer2d;
struct input_manager;
struct resource_manager;
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
    struct resource_manager    *rm;
    struct ui_manager          *uim;

    struct ui_imagebox         *logo;
    struct ui_imagebox         *label1;
    struct ui_imagebox         *label2;
    struct ui_imagebox         *label3;
    struct ui_menu             *menu;
    struct ui_imagebox         *menuicon;

    struct user_paint_panel    *upp;
    struct user_learn_panel    *ulp;
};


int app_init(void);

void app_load_resource(const char *rootpath);

void app_on_update(double dt);

void app_on_render(void);

void app_on_resize(struct window *win, int w, int h);


extern struct app g_app;


#endif /* APP_H */
