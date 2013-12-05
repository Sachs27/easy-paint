#include <stdlib.h>

#include <sf/utils.h>
#include <sf/log.h>

#include "../../input_manager.h"


static struct input_manager *input_manager = NULL;

struct input_manager *input_manager_create(struct window *win) {
    sf_list_def_t def;

    if (input_manager) {
        sf_free(input_manager);
        input_manager = NULL;
    }

    input_manager = sf_calloc(sizeof(*input_manager));
    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct ivec2);
    sf_list_init(&input_manager->mb_left_buffer, &def);
    input_manager->win            = win;

    return input_manager;
}

void input_manager_destroy(void) {
    if (input_manager == NULL) {
        return;
    }

    sf_list_destroy(&input_manager->mb_left_buffer);
    sf_free(input_manager);
    input_manager = NULL;
}

void input_manager_touch_down(int x, int y) {
    struct input_manager *im = input_manager;

    if (im->keys[KEY_MB_LEFT] != KEY_PRESS) {
        input_manager->keys[KEY_MB_LEFT] = KEY_PRESS;
    } else if (im->keys[KEY_MB_LEFT] == KEY_PRESS
        && (x != im->mouse.x || y != im->mouse.y)) {
        struct ivec2 pos;
        pos.x = x;
        pos.y = y;
        sf_list_push(&im->mb_left_buffer, &pos);
    }
    im->mouse.x = x;
    im->mouse.y = y;
}

void input_manager_touch_up(void) {
    input_manager->keys[KEY_MB_LEFT] = KEY_RELEASE;
}

void input_manager_update(void) {
    struct input_manager *im = input_manager;

    if (sf_list_cnt(&im->mb_left_buffer) > 1) {
        sf_log(SF_LOG_INFO, "input_manager cached %u positions.",
               sf_list_cnt(&im->mb_left_buffer));
    }

    sf_list_clear(&im->mb_left_buffer);
}
