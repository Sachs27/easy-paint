#ifdef ANDROID
#include <GLES/gl.h>
#else /* ANDROID */
#include <GL/glew.h>
#endif /* ANDROID */
#include <sf/utils.h>
#include <sf/log.h>

#include "app.h"
#include "user_paint_panel.h"
#include "user_learn_panel.h"
#include "ui_imagebox.h"
#include "ui_menu.h"
#include "texture.h"
#include "resource_manager.h"


struct app g_app;

static int menuicon_on_press(struct ui *ui, int x, int y) {
    ui_show((struct ui *) &g_app.menu);
    return 0;
}

static int menu_on_press(struct ui *ui, int x, int y) {
    ui_hide((struct ui *) &g_app.menu);
    return 0;
}

static void app_change_stage(int stage) {
    switch (stage) {
    case APP_STAGE_DOODLE:
        ui_hide((struct ui *) &g_app.ulp);
        ui_show((struct ui *) &g_app.upp);
        break;
    case APP_STAGE_TEACHING:
        ui_hide((struct ui *) &g_app.upp);
        ui_show((struct ui *) &g_app.ulp);
        break;
    }
}

static int menu_item_on_press(struct ui *ui, int x, int y) {
    int i = 0;
    sf_list_iter_t iter;

    if (sf_list_begin(&g_app.menu.items, &iter)) do {
        struct ui *item = *(struct ui **) sf_list_iter_elt(&iter);
        if (item == ui) {
            app_change_stage(i);
            ui_hide((struct ui *) &g_app.menu);
            break;
        }
        ++i;
    } while (sf_list_iter_next(&iter));

    return 0;
}

int app_init(const char *rootpath) {
    if (g_app.inited) {
        return 0;
    }
    g_app.inited = SF_TRUE;

    resource_manager_init(&g_app.rm, rootpath);
    renderer2d_init(&g_app.renderer2d, g_app.window->w, g_app.window->h);
    ui_manager_init(&g_app.uim);

    ui_init(&g_app.root, g_app.window->w, g_app.window->h);

    user_paint_panel_init(&g_app.upp, g_app.window->w, g_app.window->h,
                          &g_app.rm);
    ui_add_child(&g_app.root, (struct ui *) &g_app.upp, 0, 0);

    user_learn_panel_init(&g_app.ulp, g_app.window->w, g_app.window->h,
                          &g_app.rm);
    ui_add_child(&g_app.root, (struct ui *) &g_app.ulp, 0, 0);

    ui_imagebox_init(&g_app.menuicon, 0, 0,
                     resource_manager_get(&g_app.rm, RESOURCE_TEXTURE,
                     RESOURCE_TEXTURE_ICON_PARENT));
    UI_CALLBACK(&g_app.menuicon, press, menuicon_on_press);
    ui_add_child(&g_app.root, (struct ui *) &g_app.menuicon,
                 0, g_app.window->h - g_app.menuicon.ui.area.h);

    ui_imagebox_init(&g_app.logo, 0, 0,
                     resource_manager_get(&g_app.rm, RESOURCE_TEXTURE,
                     RESOURCE_TEXTURE_ICON_LOGO));

    ui_imagebox_init(&g_app.label1, 0, 0,
                     resource_manager_get(&g_app.rm, RESOURCE_TEXTURE,
                     RESOURCE_TEXTURE_ICON_LABEL1));

    ui_imagebox_init(&g_app.label2, 0, 0,
                     resource_manager_get(&g_app.rm, RESOURCE_TEXTURE,
                     RESOURCE_TEXTURE_ICON_LABEL2));

    ui_imagebox_init(&g_app.label3, 0, 0,
                     resource_manager_get(&g_app.rm, RESOURCE_TEXTURE,
                     RESOURCE_TEXTURE_ICON_LABEL3));

    ui_menu_init(&g_app.menu, 256, g_app.window->h);
    UI_CALLBACK(&g_app.menu, press, menu_on_press);
    ui_menu_set_background_color(&g_app.menu, 64, 64, 64, 250);
    ui_menu_add_item(&g_app.menu, (struct ui *) &g_app.logo);
    ui_menu_add_item(&g_app.menu, (struct ui *) &g_app.label1);
    ui_menu_add_item(&g_app.menu, (struct ui *) &g_app.label2);
    ui_menu_add_item(&g_app.menu, (struct ui *) &g_app.label3);
    {
        int i = 0;
        sf_list_iter_t iter;

        if (sf_list_begin(&g_app.menu.items, &iter)) do {
            struct ui *item = *(struct ui**) sf_list_iter_elt(&iter);
            if (i != 0) {
                UI_CALLBACK(item, press, menu_item_on_press);
            }
            ++i;
        } while (sf_list_iter_next(&iter));
    }

    ui_add_child(&g_app.root, (struct ui *) &g_app.menu, 0, 0);

    ui_manager_set_root(&g_app.uim, &g_app.root);

    ui_show((struct ui *) &g_app.root);
    ui_hide((struct ui *) &g_app.upp);
    ui_hide((struct ui *) &g_app.ulp);
    ui_hide((struct ui *) &g_app.menu);
    ui_show((struct ui *) &g_app.menuicon);

    app_change_stage(APP_STAGE_DOODLE);

    return 0;
}

void app_destory(void) {
    ui_destroy(&g_app.root);
    resource_manager_destroy(&g_app.rm);
    ui_manager_destroy(&g_app.uim);
    renderer2d_destroy(&g_app.renderer2d);

    input_manager_destroy();
    window_destroy();

    g_app.inited = SF_FALSE;
}

void app_on_resize(struct window *win, int w, int h) {
    renderer2d_resize(&g_app.renderer2d, w, h);

    ui_resize((struct ui *) &g_app.upp, w, h);
    ui_resize((struct ui *) &g_app.ulp, w, h);
    ui_move((struct ui *) &g_app.menuicon, 0, h - g_app.menuicon.ui.area.h);

    ui_resize((struct ui *) &g_app.menu, g_app.menu.ui.area.w, h);
}

#if 0
static void handle_mouse_button_right(void) {
    static int lastx, lasty;

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_PRESS) {
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

    if (g_app.im->keys[KEY_MB_RIGHT] == KEY_REPEAT
        && (g_app.im->mouse.x != lastx || g_app.im->mouse.y != lasty)) {
        canvas_offset(&g_app.upp->canvas, lastx - g_app.im->mouse.x,
                      lasty - g_app.im->mouse.y);
        lastx = g_app.im->mouse.x;
        lasty = g_app.im->mouse.y;
    }

}
#endif

void app_on_update(double dt) {
#if 0
    handle_mouse_button_right();
    if (g_app.im->keys[KEY_UP] == KEY_PRESS) {
        int x, y;
        canvas_screen_to_canvas(g_app.canvas, g_app.im->mouse.x,
                                g_app.im->mouse.y, &x, &y);
        canvas_zoom_in(g_app.canvas, x, y);
    } else if (g_app.im->keys[KEY_DOWN] == KEY_PRESS) {
        int x, y;
        canvas_screen_to_canvas(g_app.canvas, g_app.im->mouse.x,
                                g_app.im->mouse.y, &x, &y);
        canvas_zoom_out(g_app.canvas, x, y);
    }
#endif
    static int cnt = 0;
    static double elapse = 0;

    ++cnt;
    elapse += dt;
    if (elapse > 1.0) {
        sf_log(SF_LOG_INFO, "FPS: %d\n", cnt);
        cnt = 0;
        elapse -= 1.0;
    }

    input_manager_update();
    ui_manager_update(&g_app.uim, g_app.im, dt);
}

void app_on_render(void) {
    ui_manager_render(&g_app.uim, &g_app.renderer2d);
}
