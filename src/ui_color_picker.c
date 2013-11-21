#include <assert.h>
#include <math.h>

#include "ui_color_picker.h"


#define LIGHTNESS_HEIGHT 256
#define CIRCLE_RADIUS 128

/* Helper function called by hsl2rgb */
static float hue2rgb(float p, float q, float t) {
    if (t < 0) {
        t = t + 1;
    } else if (t > 1) {
        t = t - 1;
    }

    if (6.0f * t < 1.0f) {
        return p + ((q - p) * 6 * t);
    } else if (2.0f * t < 1.0f) {
        return q;
    } else if (3.0f * t < 2.0f) {
        return p + ((q - p) * 6 * (0.666666f - t));
    }

    return p;
}

static void hsl2rgb(float hsl[3], float rgb[3]) {
    if (hsl[1] == 0) {
        /* if saturation 0, give a grayscale color
         * (fill with lightness component)
         */
        rgb[0] = rgb[1] = rgb[2] = hsl[2];
    } else {
        float p, q;

        if (hsl[2] < 0.5f) {
            q = hsl[2] * (1 + hsl[1]);
        } else {
            q = hsl[2] + hsl[1] - (hsl[2] * hsl[1]);
        }
        p = 2 * hsl[2] - q;

        rgb[0] = hue2rgb(p, q, hsl[0] + 0.333333f);
        rgb[1] = hue2rgb(p, q, hsl[0]);
        rgb[2] = hue2rgb(p, q, hsl[0] - 0.333333f);
    }
}

static void update_lightness(struct ui_color_picker *cp) {
    uint32_t colors[LIGHTNESS_HEIGHT];
    uint8_t *color = (uint8_t *) (colors + LIGHTNESS_HEIGHT);
    float rgb[3], hsl[3];
    int i;

    hsl[0] = cp->h;
    hsl[1] = cp->s;
    for (i = 0; i < LIGHTNESS_HEIGHT; ++i) {
        color -= 4;

        hsl[2] = i / 255.0f;
        hsl2rgb(hsl, rgb);
        color[0] = rgb[0] * 0xFF;
        color[1] = rgb[1] * 0xFF;
        color[2] = rgb[2] * 0xFF;
        color[3] = 0xFF;
    }
    glBindTexture(GL_TEXTURE_2D, cp->lightness.tid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cp->lightness.w, cp->lightness.h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void update_circle(struct ui_color_picker *cp) {
    uint32_t colors[2 * 2 * CIRCLE_RADIUS * CIRCLE_RADIUS];
    uint8_t *color = (uint8_t *) colors;
    int d = CIRCLE_RADIUS * 2;
    int x, y;
    float hsl[3], rgb[3];

    hsl[2] = cp->l;
    for (y = 0; y < d; ++y)
    for (x = 0; x < d; ++x) {
        int dx = x - CIRCLE_RADIUS;
        int dy = y - CIRCLE_RADIUS;
        float dr = sqrt(dx * dx + dy * dy);

        if (dr > CIRCLE_RADIUS) {
            color[0] = color[1] = color[2] = color[3] = 0;
        } else {
            hsl[0] = (atan2(dy, dx) / 3.1415926f + 1.0f) / 2.0f;
            hsl[1] = dr / CIRCLE_RADIUS;

            hsl2rgb(hsl, rgb);

            color[0] = rgb[0] * 0xFF;
            color[1] = rgb[1] * 0xFF;
            color[2] = rgb[2] * 0xFF;
            color[3] = 0xFF;
        }

        color += 4;
    }

    glBindTexture(GL_TEXTURE_2D, cp->circle.tid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cp->circle.w, cp->circle.h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void ui_color_picker_on_render(struct ui *ui, struct renderer2d *r) {
    struct ui_color_picker *cp = (struct ui_color_picker *) ui;

    renderer2d_fill_rect(r, 0, 0, cp->ui.area.w, cp->ui.area.h,
                         128, 128, 128, 220);

    renderer2d_draw_texture(r, cp->lightness_area.x, cp->lightness_area.y,
                            cp->lightness_area.w, cp->lightness_area.h,
                            &cp->lightness, 0, 0, 0, 0);

    renderer2d_draw_texture(r, cp->circle_area.x, cp->circle_area.y,
                            cp->circle_area.w, cp->circle_area.h,
                            &cp->circle, 0, 0, 0, 0);

    renderer2d_fill_rect(r, cp->lightness_area.x + cp->lightness_area.w,
                         cp->lightness_area.y + cp->lightness_area.h,
                         cp->lightness_area.h / 2, cp->lightness_area.w,
                         cp->ncolor[0] * 0xFF, cp->ncolor[1] * 0xFF,
                         cp->ncolor[2] * 0xFF, 0xFF);

    renderer2d_fill_rect(r, cp->lightness_area.x + cp->lightness_area.w
                         + cp->lightness_area.h / 2,
                         cp->lightness_area.y + cp->lightness_area.h,
                         cp->lightness_area.h / 2, cp->lightness_area.w,
                         cp->ocolor[0] * 0xFF, cp->ocolor[1] * 0xFF,
                         cp->ocolor[2] * 0xFF, 0xFF);
}

static int ui_color_picker_on_press(struct ui *ui, int x, int y) {
    struct ui_color_picker *cp = (struct ui_color_picker *) ui;

    if (sf_rect_iscontain(&cp->lightness_area, x, y)) {
        cp->ispress_lightness = 1;
    } else if (sf_rect_iscontain(&cp->circle_area, x, y)) {
        cp->ispress_circle = 1;
    }

    return 0;
}

static void ui_color_picker_on_release(struct ui *ui) {
    struct ui_color_picker *cp = (struct ui_color_picker *) ui;

    cp->ispress_lightness = 0;
    cp->ispress_circle    = 0;
}

static void ui_color_picker_on_update(struct ui *ui, struct input_manager *im,
                                      double dt) {
    struct ui_color_picker *cp = (struct ui_color_picker *) ui;
    int xscreen, yscreen;
    int x, y;

    ui_get_screen_pos(ui, &xscreen, &yscreen);

    x = im->mouse.x - xscreen;
    y = im->mouse.y - yscreen;

    if (cp->ispress_lightness) {
        float l = 1.0f
                  - (y - cp->lightness_area.y) * 1.0f / cp->lightness_area.h;
        if (l < 0.0f) {
            l = 0.0f;
        } else if (l > 1.0f) {
            l = 1.0f;
        }
        if (cp->l != l) {
            cp->l = l;
            hsl2rgb(&cp->h, cp->ncolor);
            update_circle(cp);
        }
    }

    if (cp->ispress_circle) {
        int dx, dy;
        float h, s;

        dx = (x - cp->circle_area.x) - CIRCLE_RADIUS;
        dy = (y - cp->circle_area.y) - CIRCLE_RADIUS;

        h = (atan2(dy, dx) / 3.1415926f + 1.0f) / 2.0f;
        s = sqrt(dx * dx + dy * dy) / CIRCLE_RADIUS;
        if (s < 0.0f) {
            s = 0.0f;
        } else if (s > 1.0f) {
            s = 1.0f;
        }
        if (cp->h != h && cp->s != s) {
            cp->h = h;
            cp->s = s;
            hsl2rgb(&cp->h, cp->ncolor);
            update_lightness(cp);
        }
    }
}

int ui_color_picker_init(struct ui_color_picker *cp, int w, int h) {
    ui_init((struct ui *) cp, w, h);

    cp->h = 0;
    cp->s = 1.0f;
    cp->l = 0.5f;

    cp->lightness_area.x = 10;
    cp->lightness_area.y = 0;
    cp->lightness_area.w = 24;
    cp->lightness_area.h = LIGHTNESS_HEIGHT;
    texture_init_2d(&cp->lightness, 1, LIGHTNESS_HEIGHT);
    update_lightness(cp);

    cp->circle_area.x = cp->lightness_area.x + cp->lightness_area.w + 10;
    cp->circle_area.y = 0;
    cp->circle_area.w = 2 * CIRCLE_RADIUS;
    cp->circle_area.h = 2 * CIRCLE_RADIUS;
    texture_init_2d(&cp->circle, 2 * CIRCLE_RADIUS, 2 * CIRCLE_RADIUS);
    update_circle(cp);

    UI_CALLBACK(cp, render, ui_color_picker_on_render);
    UI_CALLBACK(cp, press, ui_color_picker_on_press);
    UI_CALLBACK(cp, release, ui_color_picker_on_release);
    UI_CALLBACK(cp, update, ui_color_picker_on_update);

    return 0;
}

void ui_color_picker_set_color(struct ui_color_picker *cp, float color[3]) {
    cp->ocolor[0] = color[0];
    cp->ocolor[1] = color[1];
    cp->ocolor[2] = color[2];
    cp->ncolor[0] = color[0];
    cp->ncolor[1] = color[1];
    cp->ncolor[2] = color[2];
}

void ui_color_picker_get_color(struct ui_color_picker *cp, float color[3]) {
    color[0] = cp->ncolor[0];
    color[1] = cp->ncolor[1];
    color[2] = cp->ncolor[2];
}
